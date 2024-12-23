#include "parser.h"

CST* parseTokenTable(char* filename, tokenTable* table){
    if(!filename || !table){
        fprintf(stderr, "Error parseTokenTable: no file name or token table.\n");
        exit(1);
    }
    CST* cst = calloc(1,sizeof(CST));
    if(!cst){
        fprintf(stderr, "Error parseTokenTable @%s: not enough memory, cannot create parse tree.\n", filename);
        exit(1);
    }
    cst->root = createTreeNode(filename, NULL);
    
    printf("Debug: in expressions only mode.\n");
    while(hasNext(table)){
        parseExpression(cst->root, table);
    }
    return cst;
}

void parseTerm(treeNode* parent, tokenTable* table){
    treeNode* term = insertNewNode2Parent("term", NULL, parent);
    
    parseBaseTerm(term, table);
    //printCurrentToken(table);
    
    tokenNode* peeknext = peekNextNode(table);
    /*
    if(peeknext){
        printf("Debug: next node is %d\n", peeknext->t->type);
    }else{
        printf("Debug: reached EOF\n");
    }
    */
    // use while loop to accomplish chaining accessing: obj.method1().field1.field2[2].method2()
    while(peeknext){
        if(peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='.'){
            //printf("Debug: parse field access\n");
            parseFieldAccess(term, table);
        }
        else if(peeknext->t->type==BRACKET && peeknext->t->data.char_val=='['){
            //printf("Debug: parse array access\n");
            parseArrayAccess(term, table);
        }
        else if(peeknext->t->type==BRACKET && peeknext->t->data.char_val=='('){
            //printf("Debug: parse subroutine call\n");
            parseSubroutineCall(term, table);
        }else{
            break; // there is no more chained access
        }
        peeknext = peekNextNode(table);
    }

}

void parseBaseTerm(treeNode* parent, tokenTable* table){
    tokenNode* n = nextNode(table);
    if(!n){
        fprintf(stderr, "Error parseBaseTerm: unexpected end of tokens .\n");
        exit(1);
    }
    // four terminal cases: true, false, null, this
    if(n->t->type==KEYWORD && (n->t->data.key_val==BOOL_TRUE || n->t->data.key_val==BOOL_FALSE || n->t->data.key_val==NULLER || n->t->data.key_val==THIS)){
        insertNewNode2Parent("keyword", n->t, parent);
    }
    // token is a number
    else if(n->t->type==NUMBER){
        insertNewNode2Parent("number", n->t, parent);
    }
    // token is a string
    else if(n->t->type==STRING){
        insertNewNode2Parent("string", n->t, parent);
    }
    // token is a variable 
    else if(n->t->type==IDENTIFIER){
        insertNewNode2Parent("identifier", n->t, parent);
    }
    // term is an expression within a round bracket
    else if(n->t->type==BRACKET && n->t->data.char_val=='('){
        treeNode* parenthesized = insertNewNode2Parent("parenthesizedExpression", NULL, parent);
        insertNewNode2Parent("bracket", n->t, parenthesized);
        
        parseExpression(parenthesized, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseBaseTerm", "missing right parenthesis");
        insertNewNode2Parent("bracket", n->t, parenthesized);
    }
    else{
        fprintf(stderr, "Error parseBaseTerm line %d: invalid base term.\n", n->t->lineNumber);
        exit(1);
    }
}

void parseFieldAccess(treeNode* parent, tokenTable* table){
    treeNode* field = insertNewNode2Parent("fieldAccess", NULL, parent);
    
    // dot
    tokenNode* n = nextNode(table);
    insertNewNode2Parent("symbol", n->t, field);
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseFieldAccess", "identifier expected after dot");
    insertNewNode2Parent("identifier", n->t, field);
}

void parseArrayAccess(treeNode* parent, tokenTable* table){
    treeNode* array = insertNewNode2Parent("arrayAccess", NULL, parent);
    
    // left bracket
    tokenNode* n = nextNode(table);
    insertNewNode2Parent("bracket", n->t, array);
    
    // expression
    parseExpression(array, table);
    
    // right bracket
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ']', "parseArrayAccess", "missing right bracket");
    insertNewNode2Parent("bracket", n->t, array);
}

void parseSubroutineCall(treeNode* parent, tokenTable* table){
    treeNode* call = insertNewNode2Parent("subroutineCall", NULL, parent);
    
    // left parenthesis
    tokenNode* n = nextNode(table);
    insertNewNode2Parent("bracket", n->t, call);
    
    // argument list
    parseExpressionList(call, table);
    
    // right parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseSubroutineCall", "missing right parenthesis");
    insertNewNode2Parent("bracket", n->t, call);
}

void parseExpressionList(treeNode* parent, tokenTable* table){
    treeNode* list = insertNewNode2Parent("expressionList", NULL, parent);
    
    tokenNode* peeknext = peekNextNode(table);
    if(!peeknext || !isExpressionStart(peeknext->t)){
        return; // empty argument list.
    }
    
    parseExpression(list, table);
    
    // deal with zero or more arguments
    peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && peeknext->t->data.char_val==','){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, list);
        
        parseExpression(list, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseExpression(treeNode* parent, tokenTable* table){
    treeNode* exp = insertNewNode2Parent("expression", NULL, parent);
    
    parseTernaryExpression(exp, table);
}

void parseTernaryExpression(treeNode* parent, tokenTable* table){
    treeNode* ternary = insertNewNode2Parent("ternaryExpression", NULL, parent);
    
    parseLogicalOrExpression(ternary, table);
    
    tokenNode* peeknext = peekNextNode(table);
    if(!peeknext || peeknext->t->type!=SYMBOL || peeknext->t->data.char_val!='?'){
        return; // no ternary expression, this reduces to a simple expression.
    }
    
    // '?'
    tokenNode* n = nextNode(table);
    insertNewNode2Parent("symbol", n->t, ternary);
    
    parseExpression(ternary, table);
    
    // ':'   
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, ':', "parseTernaryExpression", "missing colon in ternary expression");
    insertNewNode2Parent("symbol", n->t, ternary);
    
    parseExpression(ternary, table);
}

void parseLogicalOrExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalOr = insertNewNode2Parent("logicalOrExpression", NULL, parent);
    
    parseLogicalAndExpression(logicalOr, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==OPERATOR && !strcmp(peeknext->t->data.str_val, "||")){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, logicalOr);
        
        parseLogicalAndExpression(logicalOr, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseLogicalAndExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalAnd = insertNewNode2Parent("logicalAndExpression", NULL, parent);
    
    parseBitwiseOrExpression(logicalAnd, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==OPERATOR && !strcmp(peeknext->t->data.str_val, "&&")){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, logicalAnd);
        
        parseBitwiseOrExpression(logicalAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseBitwiseOrExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseOr = insertNewNode2Parent("bitwiseOrExpression", NULL, parent);
    
    parseBitwiseXorExpression(bitwiseOr, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='|'){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, bitwiseOr);
        
        parseBitwiseXorExpression(bitwiseOr, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseBitwiseXorExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseXor = insertNewNode2Parent("bitwiseXorExpression", NULL, parent);
    
    parseBitwiseAndExpression(bitwiseXor, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='^'){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, bitwiseXor);
        
        parseBitwiseAndExpression(bitwiseXor, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseBitwiseAndExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseAnd = insertNewNode2Parent("bitwiseAndExpression", NULL, parent);
    
    parseEqualityExpression(bitwiseAnd, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='&'){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, bitwiseAnd);
        
        parseEqualityExpression(bitwiseAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseEqualityExpression(treeNode* parent, tokenTable* table){
    treeNode* equality = insertNewNode2Parent("equalityExpression", NULL, parent);
    
    parseRelationalExpression(equality, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->data.str_val, "==") || !strcmp(peeknext->t->data.str_val, "!=")) ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, equality);
        
        parseRelationalExpression(equality, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseRelationalExpression(treeNode* parent, tokenTable* table) {
    treeNode* relational = insertNewNode2Parent("relationalExpression", NULL, parent);

    parseShiftExpression(relational, table);

    tokenNode* peeknext = peekNextNode(table);
    
    // Check for '<', '>', '<=', '>=', or 'instanceof'
    if (peeknext && peeknext->t->type == SYMBOL && 
        (peeknext->t->data.char_val == '<' || peeknext->t->data.char_val == '>')) {
        //printf("Debug: verify generics.\n");
        if(isPotentialGenerics(peeknext)){
            //printf("Debug: valid generics detected.\n");
            parseGenerics(relational, table);
            return;
        }
    }   
    //printf("Debug: invalid generics\n");

    // If we reach here, it's not generics; process as relational operator
    peeknext = peekNextNode(table);
    if (peeknext && peeknext->t->type == SYMBOL && 
        (peeknext->t->data.char_val == '<' || peeknext->t->data.char_val == '>')) {
        tokenNode* peeknextnext = peeknext->next;
        if (peeknextnext && peeknextnext->t->type == SYMBOL && peeknextnext->t->data.char_val == '=') {
            // Combine '<=' or '>='
            char combined[3] = {peeknext->t->data.char_val, peeknextnext->t->data.char_val, 0};
            peeknext->t->type = OPERATOR;
            peeknext->t->data.str_val = calloc(3, sizeof(char));
            if (!peeknext->t->data.str_val) {
                fprintf(stderr, "Error parseRelationalExpression: not enough memory, cannot create a string value for the node\n");
                exit(1);
            }
            strcpy(peeknext->t->data.str_val, combined);

            // Remove the second token
            tokenNode* tmp = peeknextnext;
            peeknext->next = tmp->next;
            if (tmp->next) {
                tmp->next->prev = peeknext;
            } else {
                table->end = peeknext;
            }
            freeToken(tmp->t);
            free(tmp);
        }
        //printf("Debug: finished combining symbols\n");
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, relational);
        //printf("Debug: inserted combined operator\n");
        //printf("--------------\n");
        //printTokenTable(table);
        //printf("--------------\n");
        //printCurrentToken(peekNextNode(table));
        parseShiftExpression(relational, table);
    }        
    else if (peeknext && peeknext->t->type == KEYWORD && peeknext->t->data.key_val == INSTANCEOF) {
    // Handle instanceof keyword
    tokenNode* n = nextNode(table);
    insertNewNode2Parent("keyword", n->t, relational);
    parseReferenceType(relational, table);
    }
}


void parseShiftExpression(treeNode* parent, tokenTable* table){
    treeNode* shift = insertNewNode2Parent("shiftExpression", NULL, parent);
    
    parseAdditiveExpression(shift, table);
    
    tokenNode* peeknext = peekNextNode(table);
    tokenNode* peeknextnext = peeknext ? peeknext->next : NULL;
    
    while(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->data.char_val=='>' || peeknext->t->data.char_val=='<') && peeknextnext && peeknextnext->t->type==SYMBOL && peeknextnext->t->data.char_val==peeknext->t->data.char_val ){
        // modify the token table, combine the two nodes into one
        char combined[3] = {peeknext->t->data.char_val, peeknextnext->t->data.char_val, 0};
        peeknext->t->type = OPERATOR;
        peeknext->t->data.str_val = calloc(3,sizeof(char));
        if(!peeknext->t->data.str_val){
            fprintf(stderr, "Error parseShiftExpression: not enough memory, cannot create a string value for the node\n");
            exit(1);
        }
        strcpy(peeknext->t->data.str_val, combined);
        peeknext->next = peeknextnext->next;
        if(peeknextnext->next){
            peeknextnext->next->prev = peeknext;
        }else{
            table->end = peeknext;
        }
        freeToken(peeknextnext->t);
        free(peeknextnext);
            
        // now we can add this node to the tree.
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, shift);
        
        parseAdditiveExpression(shift, table);
        
        peeknext = peekNextNode(table);
        peeknextnext = peeknext ? peeknext->next : NULL;
    } 
}

void parseAdditiveExpression(treeNode* parent, tokenTable* table){
    treeNode* additive = insertNewNode2Parent("additiveExpression", NULL, parent);
    
    parseMultiplicativeExpression(additive, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->data.char_val=='+' || peeknext->t->data.char_val=='-') ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, additive);
        
        parseMultiplicativeExpression(additive, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseMultiplicativeExpression(treeNode* parent, tokenTable* table){
    treeNode* multiplicative = insertNewNode2Parent("multiplicativeExpression", NULL, parent);
    
    parseUnaryExpression(multiplicative, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->data.char_val=='*' || peeknext->t->data.char_val=='/' || peeknext->t->data.char_val=='%') ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, multiplicative);
        
        parseUnaryExpression(multiplicative, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseUnaryExpression(treeNode* parent, tokenTable* table){
    treeNode* unary = insertNewNode2Parent("unaryExpression", NULL, parent);
    
    tokenNode* peeknext = peekNextNode(table);
    if(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->data.char_val=='!' || peeknext->t->data.char_val=='-' || peeknext->t->data.char_val=='~') ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, unary);
        
        parseUnaryExpression(unary, table);

        return;
    }
    else if(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->data.str_val, "++") || !strcmp(peeknext->t->data.str_val, "--")) ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, unary);
        
        parseTerm(unary, table);

        return;
    }
    else{
        parsePostfixExpression(unary, table);
    }
}

void parsePostfixExpression(treeNode* parent, tokenTable* table){
    treeNode* postfix = insertNewNode2Parent("postfixExpression", NULL, parent);
    
    parseTerm(postfix, table);
    
    tokenNode* peeknext = peekNextNode(table);
    
    if(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->data.str_val, "++") || !strcmp(peeknext->t->data.str_val, "--")) ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, postfix);
    }
}

void parseType(treeNode* parent, tokenTable* table){
    treeNode* type = insertNewNode2Parent("type", NULL, parent);
    
    tokenNode* peeknext = peekNextNode(table);
    if(peeknext && peeknext->t->type==KEYWORD && (peeknext->t->data.key_val==CHAR || peeknext->t->data.key_val==INT || peeknext->t->data.key_val==LONG || peeknext->t->data.key_val==DOUBLE || peeknext->t->data.key_val==BOOLEAN)){
        
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("primitiveType", n->t, type);
        
        return;
    }
    
    parseReferenceType(type, table);
    
}

void parseReferenceType(treeNode* parent, tokenTable* table){
    treeNode* reference = insertNewNode2Parent("referenceType", NULL, parent);
    
    tokenNode* n = nextNode(table);
    if(!n){
        fprintf(stderr, "Error parseReferenceType: unexpected end of tokens\n");
        exit(1);
    }
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseReferenceType", "missing identifier for reference type");
    insertNewNode2Parent("identifier", n->t, reference);
    
    tokenNode* peeknext = peekNextNode(table);
    if(peeknext && peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='<'){
        parseGenerics(reference, table);
    }
}

void parseGenerics(treeNode* parent, tokenTable* table){
    treeNode* generics = insertNewNode2Parent("generics", NULL, parent);
    
    //printf("Debug: begin parsing generics\n");
    
    // use depth to mimic stack
    int depth = 0;
    
    // '<'
    tokenNode* n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, '<', "parseGenerics", "missing '<' to start generics");
    insertNewNode2Parent("langle", n->t, generics);
    depth++;
    
    while(depth > 0){
        tokenNode* peeknext = peekNextNode(table);
        if(!peeknext){
            fprintf(stderr, "Error parseGenerics: unexpected end of tokens inside generics\n");
            exit(1);
        }
        
        if(peeknext->t->type==SYMBOL && peeknext->t->data.char_val!='?'){
            if(peeknext->t->data.char_val=='<'){
                n = nextNode(table);
                insertNewNode2Parent("langle", n->t, generics);
                depth++;
            }else if(peeknext->t->data.char_val=='>'){
                n = nextNode(table);
                insertNewNode2Parent("rangle", n->t, generics);
                depth--;
            }else if(peeknext->t->data.char_val==','){
                n = nextNode(table);
                insertNewNode2Parent("delimiter", n->t, generics);
            }else{
                fprintf(stderr, "Error parseGenerics: unexpected symbol %c in generics\n", peeknext->t->data.char_val);
                exit(1);
            }
        }else if(peeknext->t->type==IDENTIFIER || (peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='?') ){
            parseTypeArgument(generics, table);
        }else {
            fprintf(stderr, "Error parseGenerics: unexpected token type %d in generics\n", peeknext->t->type);
            exit(1);
        }
        
    }
    //printf("Debug: end parsing generics\n");
    //printTokenTable(table);
    
}

void parseTypeArgument(treeNode* parent, tokenTable* table){
    treeNode* typeArgument = insertNewNode2Parent("typeArgument", NULL, parent);
    
    tokenNode* peeknext = peekNextNode(table);
    if(!peeknext){
        fprintf(stderr, "Error parseTypeArgument: unexpected end of tokens\n");
        exit(1);
    }
    
    if(peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='?'){
        // '?'
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("wildcard", n->t, typeArgument); 
        
        peeknext = peekNextNode(table);
        if(peeknext && peeknext->t->type==KEYWORD && (peeknext->t->data.key_val==EXTENDS || peeknext->t->data.key_val==SUPER) ){
            // extends or super
            n = nextNode(table);
            insertNewNode2Parent("keyword", n->t, typeArgument);
            
            parseReferenceType(typeArgument, table);
        }
    }
    else if(peeknext->t->type==IDENTIFIER){
        parseReferenceType(typeArgument, table);
    }else{
        fprintf(stderr, "Error parseTypeArgument: invalid token in type arguments\n");
        exit(1);
    }
}

void parseAssignment(treeNode* parent, tokenTable* table){
    treeNode* assignment = insertNewNode2Parent("assignment", NULL, parent);
    
    // a mandatory identifier for the variable
    tokenNode* n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseAssignment", "missing starting identifier for an assignment");
    insertNewNode2Parent("identifier", n->t, assignment); 
    
    // an optional array access
    tokenNode* peeknext = peekNextNode(table);
    if(isBracket('[', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("bracket", n->t, assignment); 
        
        parseExpression(assignment, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ']', "parseAssignment", "missing right square bracket to conclude array access");
        insertNewNode2Parent("bracket", n->t, assignment); 
    }
    
    // a mandatory assignment operator
    n = nextNode(table);
    if!((isSymbol('=', n) || isOperator("+=", n) || isOperator("-=", n) || isOperator("*=", n) || isOperator("/=", n))){
        fprintf("Error parseAssignment line %d: missing assignment operator\n", n->t->lineNumber);
        exit(1);
    }
    insertNewNode2Parent("assignmentOperator", n->t, assignment); 
    
    // an expression
    
    parseExpression(assignment, table);
}

void parseVariableDeclaration(treeNode* parent, tokenTable* table);
void parseSubroutineDeclaration(treeNode* parent, tokenTable* table);
void parseParameterList(treeNode* parent, tokenTable* table);
void parseSubroutineBody(treeNode* parent, tokenTable* table);


