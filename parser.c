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
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isKey(NEW, peeknext)){
        parseNewObject(term, table);
    }else if(isBracket('{', peeknext)){
        parseArrayInitialization(term, table);
    }else{
        parseBaseTerm(term, table);
    }
    
    peeknext = peekNextNode(table);

    // use while loop to accomplish chaining accessing: obj.method1().field1.field2[2].method2()
    while(peeknext){
        if(isSymbol('.', peeknext)){
            parseFieldAccess(term, table);
        }
        else if(isBracket('[', peeknext)){
            parseArrayAccess(term, table);
        }
        else if(isBracket('(', peeknext)){
            parseSubroutineCall(term, table);
        }else{
            break; // there is no more chained access
        }
        peeknext = peekNextNode(table);
    }

}

void parseNewObject(treeNode* parent, tokenTable* table){
    treeNode* newObject = insertNewNode2Parent("newObject", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // 'new' keyword
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, NEW, "parseNewObject", "missng 'new' keyword to create a new object");
    insertNewNode2Parent("new", n->t, newObject);
    
    // type
    parseType(newObject, table);
    
    // optional generics
    peeknext = peekNextNode(table);
    if(isBracket('<', peeknext)){
        parseGenerics(newObject, table);
    }
    
    // branch: constructor or array
    peeknext = peekNextNode(table);
    if(isBracket('(', peeknext)){
        // constructor call
        n = nextNode(table);
        insertNewNode2Parent("bracket", n->t, newObject);
        
        parseExpressionList(newObject, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseNewObject", "missng right parenthesis to conclude argument list");
        insertNewNode2Parent("bracket", n->t, newObject);
    }
    else if(isBracket('[', peeknext)){
        // array
        // let's defer the rule, that int[10][] is valid while int[][10] is invalid to semantics.
        
        while(isBracket('[', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent("bracket", n->t, newObject);
            
            parseExpression(newObject, table);
            
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, ']', "parseNewObject", "missng right square bracket to conclude array length");
            insertNewNode2Parent("bracket", n->t, newObject);
            peeknext = peekNextNode(table);
        }
        
        // optional initialization
        peeknext = peekNextNode(table);
        if(isBracket('{', peeknext)){
            parseArrayInitialization(newObject, table);
        }
    }else{
        // the new object expression must either be a constructor call, or an array
        fprintf(stderr, "Error parseNewObject line %d: missing constructor call or array initialization for a new object\n", peeknext->t->lineNumber);
        exit(1);
    }
}

void parseArrayInitialization(treeNode* parent, tokenTable* table){
    treeNode* arrayInitialization = insertNewNode2Parent("arrayInitialization", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "arrayInitialization", "missng left brace to start array initialization");
    insertNewNode2Parent("bracket", n->t, arrayInitialization);
    
    peeknext = peekNextNode(table);
    if(isBracket('{', peeknext)){
        parseArrayInitialization(arrayInitialization, table);
        peeknext = peekNextNode(table);
        while(isSymbol(',', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent("symbol", n->t, arrayInitialization);
            parseArrayInitialization(arrayInitialization, table);
        }
    }else{
        parseTerm(arrayInitialization, table);
        peeknext = peekNextNode(table);
        while(isSymbol(',', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent("symbol", n->t, arrayInitialization);
            parseTerm(arrayInitialization, table);
        }
    }
}

void parseBaseTerm(treeNode* parent, tokenTable* table){
    tokenNode* n;
    tokenNode* peeknext = nextNode(table);
    if(!peeknext){
        fprintf(stderr, "Error parseBaseTerm: unexpected end of tokens .\n");
        exit(1);
    }
    // four terminal cases: true, false, null, this
    if(isKey(BOOL_TRUE, peeknext) || isKey(BOOL_FALSE, peeknext) || isKey(NULLER, peeknext) || isKey(THIS, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("keyword", n->t, parent);
    }
    // token is a number
    else if(isNumber(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("number", n->t, parent);
    }
    // token is a string
    else if(isString(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("string", n->t, parent);
    }
    // token is a variable 
    else if(isIdentifier(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("identifier", n->t, parent);
    }
    // term is an expression within a round bracket
    else if(isBracket('(', peeknext)){
        n = nextNode(table);
        treeNode* parenthesized = insertNewNode2Parent("parenthesizedExpression", NULL, parent);
        insertNewNode2Parent("bracket", n->t, parenthesized);
        
        parseExpression(parenthesized, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseBaseTerm", "missing right parenthesis");
        insertNewNode2Parent("bracket", n->t, parenthesized);
    }
    /*
    else{
        fprintf(stderr, "Error parseBaseTerm line %d: invalid base term.\n", peeknext->t->lineNumber);
        exit(1);
    }
    */
}

void parseFieldAccess(treeNode* parent, tokenTable* table){
    treeNode* field = insertNewNode2Parent("fieldAccess", NULL, parent);
    tokenNode* n;
    
    // dot
    n = nextNode(table);
    insertNewNode2Parent("symbol", n->t, field);
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseFieldAccess", "identifier expected after dot");
    insertNewNode2Parent("identifier", n->t, field);
}

void parseArrayAccess(treeNode* parent, tokenTable* table){
    treeNode* array = insertNewNode2Parent("arrayAccess", NULL, parent);
    tokenNode* n;
    
    // left bracket
    n = nextNode(table);
    insertNewNode2Parent("bracket", n->t, array);
    
    // expression
    parseExpression(array, table);
    
    // right bracket
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ']', "parseArrayAccess", "missing right square bracket to conclude array access");
    insertNewNode2Parent("bracket", n->t, array);
}

void parseSubroutineCall(treeNode* parent, tokenTable* table){
    treeNode* call = insertNewNode2Parent("subroutineCall", NULL, parent);
    tokenNode* n;

    // left parenthesis
    n = nextNode(table);
    insertNewNode2Parent("bracket", n->t, call);
    
    // argument list
    parseExpressionList(call, table);
    
    // right parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseSubroutineCall", "missing right parenthesis to conclude subroutine call");
    insertNewNode2Parent("bracket", n->t, call);
}

void parseExpressionList(treeNode* parent, tokenTable* table){
    treeNode* list = insertNewNode2Parent("expressionList", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(!peeknext || !isExpressionStart(peeknext)){
        return; // empty argument list.
    }
    
    parseExpression(list, table);
    
    // deal with zero or more arguments
    peeknext = peekNextNode(table);
    while(isSymbol(',', peeknext)){
        n = nextNode(table);
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
    tokenNode* n;
    tokenNode* peeknext;
    
    parseLogicalOrExpression(ternary, table);
    
    peeknext = peekNextNode(table);
    if(!isSymbol('?', peeknext)){
        return; // no ternary expression, this reduces to a simple expression.
    }
    
    // '?'
    n = nextNode(table);
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
    tokenNode* n;
    tokenNode* peeknext;
    
    parseLogicalAndExpression(logicalOr, table);
    
    peeknext = peekNextNode(table);
    while(isOperator("||", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("operator", n->t, logicalOr);
        
        parseLogicalAndExpression(logicalOr, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseLogicalAndExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalAnd = insertNewNode2Parent("logicalAndExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseBitwiseOrExpression(logicalAnd, table);
    
    peeknext = peekNextNode(table);
    while(isOperator("&&", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("operator", n->t, logicalAnd);
        
        parseBitwiseOrExpression(logicalAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseBitwiseOrExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseOr = insertNewNode2Parent("bitwiseOrExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseBitwiseXorExpression(bitwiseOr, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('|', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, bitwiseOr);
        
        parseBitwiseXorExpression(bitwiseOr, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseBitwiseXorExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseXor = insertNewNode2Parent("bitwiseXorExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseBitwiseAndExpression(bitwiseXor, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('^', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, bitwiseXor);
        
        parseBitwiseAndExpression(bitwiseXor, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseBitwiseAndExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseAnd = insertNewNode2Parent("bitwiseAndExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseEqualityExpression(bitwiseAnd, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('&', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, bitwiseAnd);
        
        parseEqualityExpression(bitwiseAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseEqualityExpression(treeNode* parent, tokenTable* table){
    treeNode* equality = insertNewNode2Parent("equalityExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseRelationalExpression(equality, table);
    
    peeknext = peekNextNode(table);
    while(isOperator("==", peeknext) || isOperator("!=", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("operator", n->t, equality);
        
        parseRelationalExpression(equality, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseRelationalExpression(treeNode* parent, tokenTable* table) {
    treeNode* relational = insertNewNode2Parent("relationalExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseShiftExpression(relational, table);

    peeknext = peekNextNode(table);
    
    // Check for '<', '>', '<=', '>=', or 'instanceof'
    if (isSymbol('<', peeknext) || isSymbol('>', peeknext)) {
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
    if (isSymbol('<', peeknext) || isSymbol('>', peeknext)) {
        tokenNode* peeknextnext = peeknext->next;
        if (isSymbol('=', peeknext)) {
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
        n = nextNode(table);
        insertNewNode2Parent("operator", n->t, relational);
        //printf("Debug: inserted combined operator\n");
        //printf("--------------\n");
        //printTokenTable(table);
        //printf("--------------\n");
        //printCurrentToken(peekNextNode(table));
        parseShiftExpression(relational, table);
    }        
    else if (isKey(INSTANCEOF, peeknext)) {
    // Handle instanceof keyword
    tokenNode* n = nextNode(table);
    insertNewNode2Parent("keyword", n->t, relational);
    parseReferenceType(relational, table);
    }
}


void parseShiftExpression(treeNode* parent, tokenTable* table){
    treeNode* shift = insertNewNode2Parent("shiftExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseAdditiveExpression(shift, table);
    
    peeknext = peekNextNode(table);
    tokenNode* peeknextnext = peeknext ? peeknext->next : NULL;
    
    while( ( isSymbol('>', peeknext) || isSymbol('<', peeknext) ) && isSymbol(peeknext->t->data.char_val, peeknextnext) ){
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
        n = nextNode(table);
        insertNewNode2Parent("operator", n->t, shift);
        
        parseAdditiveExpression(shift, table);
        
        peeknext = peekNextNode(table);
        peeknextnext = peeknext ? peeknext->next : NULL;
    } 
}

void parseAdditiveExpression(treeNode* parent, tokenTable* table){
    treeNode* additive = insertNewNode2Parent("additiveExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseMultiplicativeExpression(additive, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('+', peeknext) || isSymbol('-', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, additive);
        
        parseMultiplicativeExpression(additive, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseMultiplicativeExpression(treeNode* parent, tokenTable* table){
    treeNode* multiplicative = insertNewNode2Parent("multiplicativeExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseCastExpression(multiplicative, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('*', peeknext) || isSymbol('/', peeknext) || isSymbol('%', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, multiplicative);
        
        parseCastExpression(multiplicative, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseCastExpression(treeNode* parent, tokenTable* table){
    treeNode* cast = insertNewNode2Parent("castExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isBracket('(', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("bracket", n->t, cast);
        
        parseType(cast, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseCastExpression", "missing right parenthesis to conclude casting");
        insertNewNode2Parent("bracket", n->t, cast);
    }else{
        parseUnaryExpression(cast, table);
    }
}

void parseUnaryExpression(treeNode* parent, tokenTable* table){
    treeNode* unary = insertNewNode2Parent("unaryExpression", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isSymbol('!', peeknext) || isSymbol('-', peeknext) || isSymbol('~', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, unary);
        
        parseUnaryExpression(unary, table);

        return;
    }
    else if(isOperator("++", peeknext) || isOperator("--", peeknext)){
        n = nextNode(table);
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
    tokenNode* n;
    tokenNode* peeknext;
    
    parseTerm(postfix, table);
    
    peeknext = peekNextNode(table);
    if(isOperator("++", peeknext) || isOperator("--", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("operator", n->t, postfix);
    }
}

void parseType(treeNode* parent, tokenTable* table){
    treeNode* type = insertNewNode2Parent("type", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isKey(CHAR, peeknext) || isKey(INT, peeknext) || isKey(BOOLEAN, peeknext) || isKey(LONG, peeknext) || isKey(DOUBLE, peeknext)){
        
        n = nextNode(table);
        insertNewNode2Parent("primitiveType", n->t, type);
        
        return;
    }
    
    parseReferenceType(type, table);
    
}

void parseReferenceType(treeNode* parent, tokenTable* table){
    treeNode* reference = insertNewNode2Parent("referenceType", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    n = nextNode(table);
    if(!n){
        fprintf(stderr, "Error parseReferenceType: unexpected end of tokens\n");
        exit(1);
    }
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseReferenceType", "missing identifier for reference type");
    insertNewNode2Parent("identifier", n->t, reference);
    
    peeknext = peekNextNode(table);
    if(isSymbol('<', peeknext)){
        parseGenerics(reference, table);
    }
}

void parseGenerics(treeNode* parent, tokenTable* table){
    treeNode* generics = insertNewNode2Parent("generics", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    //printf("Debug: begin parsing generics\n");
    
    // use depth to mimic stack
    int depth = 0;
    
    // '<'
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, '<', "parseGenerics", "missing '<' to start generics");
    insertNewNode2Parent("langle", n->t, generics);
    depth++;
    
    while(depth > 0){
        peeknext = peekNextNode(table);
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
        }else if(isIdentifier(peeknext) || isSymbol('?', peeknext)){
            parseTypeArgument(generics, table);
        }else {
            fprintf(stderr, "Error parseGenerics: unexpected token or end of tokens in generics\n");
            exit(1);
        }
        
    }
    //printf("Debug: end parsing generics\n");
    //printTokenTable(table);
    
}

void parseTypeArgument(treeNode* parent, tokenTable* table){
    treeNode* typeArgument = insertNewNode2Parent("typeArgument", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(!peeknext){
        fprintf(stderr, "Error parseTypeArgument: unexpected end of tokens\n");
        exit(1);
    }
    
    if(isSymbol('?', peeknext)){
        // '?'
        n = nextNode(table);
        insertNewNode2Parent("wildcard", n->t, typeArgument); 
        
        peeknext = peekNextNode(table);
        if(isKey(EXTENDS, peeknext) || isKey(SUPER, peeknext)){
            // extends or super
            n = nextNode(table);
            insertNewNode2Parent("keyword", n->t, typeArgument);
            
            parseReferenceType(typeArgument, table);
        }
    }
    else if(isIdentifier(peeknext)){
        parseReferenceType(typeArgument, table);
    }else{
        fprintf(stderr, "Error parseTypeArgument: invalid token in type arguments\n");
        exit(1);
    }
}

void parseAssignment(treeNode* parent, tokenTable* table){
    treeNode* assignment = insertNewNode2Parent("assignment", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // a mandatory identifier for the variable
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseAssignment", "missing starting identifier for an assignment");
    insertNewNode2Parent("identifier", n->t, assignment); 
    
    // an optional array access
    peeknext = peekNextNode(table);
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
    if(!(isSymbol('=', n) || isOperator("+=", n) || isOperator("-=", n) || isOperator("*=", n) || isOperator("/=", n))){
        fprintf(stderr, "Error parseAssignment line %d: missing assignment operator\n", n->t->lineNumber);
        exit(1);
    }
    insertNewNode2Parent("assignmentOperator", n->t, assignment); 
    
    // an expression
    
    parseExpression(assignment, table);
}

void parseVariableDeclaration(treeNode* parent, tokenTable* table){
    treeNode* variableDeclaration = insertNewNode2Parent("variableDeclaration", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // an optional access modifier
    peeknext = peekNextNode(table);
    if(isKey(PUBLIC, peeknext) || isKey(PRIVATE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("accessModifier", n->t, variableDeclaration); 
    }
    
    // zero or more non-access modifier
    peeknext = peekNextNode(table);
    while(isKey(STATIC, peeknext) || isKey(FINAL, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("nonAccessModifier", n->t, variableDeclaration);
    }
    
    // a mandatory type
    parseType(variableDeclaration, table);
    
    // an optional array definition
    peeknext = peekNextNode(table);
    if(isBracket('[', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("bracket", n->t, variableDeclaration);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ']', "parseVariableDeclaration", "missing right square bracket to conclude array definition");
        insertNewNode2Parent("bracket", n->t, variableDeclaration);
    }
    
    // an identifier, or an assignment
    peeknext = peekNextNode(table);
    if(isPotentialAssignment(peeknext)){
        parseAssignment(variableDeclaration, table);
    }else{
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseVariableDeclaration", "missing identifier for the variable");
        insertNewNode2Parent("identifier", n->t, variableDeclaration);
    }
    
    // zero or more definitions in the same line separated by comma
    peeknext = peekNextNode(table);
    while(isSymbol(',', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, variableDeclaration);
        peeknext = peekNextNode(table);
        if(isPotentialAssignment(peeknext)){
            parseAssignment(variableDeclaration, table);
        }else{
            n = nextNode(table);
            checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseVariableDeclaration", "missing identifier for the variable");
            insertNewNode2Parent("identifier", n->t, variableDeclaration);
        }
        peeknext = peekNextNode(table);
    }
}

void parseSubroutineDeclaration(treeNode* parent, tokenTable* table){
    treeNode* subroutineDeclaration = insertNewNode2Parent("subroutineDeclaration", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // an optional access modifier
    peeknext = peekNextNode(table);
    if(isKey(PUBLIC, peeknext) || isKey(PRIVATE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("accessModifier", n->t, subroutineDeclaration); 
    }
    
    // zero or more non-access modifier
    peeknext = peekNextNode(table);
    while(isKey(STATIC, peeknext) || isKey(FINAL, peeknext) || isKey(ABSTRACT, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("nonAccessModifier", n->t, subroutineDeclaration);
    }
    
    // an optional native
    peeknext = peekNextNode(table);
    if(isKey(NATIVE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("native", n->t, subroutineDeclaration); 
    }
    
    // a mandatory type or void
    peeknext = peekNextNode(table);
    if(isKey(VOID, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("type", n->t, subroutineDeclaration);
    }else{
        parseType(subroutineDeclaration, table);
    }
    
    // an identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseSubroutineDeclaration", "missing identifier for the method");
    insertNewNode2Parent("identifier", n->t, subroutineDeclaration);
    
    // a mandatory left parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseSubroutineDeclaration", "missing left parenthesis to start the argument list");
    insertNewNode2Parent("bracket", n->t, subroutineDeclaration);
    
    // parameter list
    parseParameterList(subroutineDeclaration, table);
    
    // a mandatory right parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseSubroutineDeclaration", "missing right parenthesis to conclude the argument list");
    insertNewNode2Parent("bracket", n->t, subroutineDeclaration);
    
    // a mandatory left brace
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseSubroutineDeclaration", "missing left brace to start the method body");
    insertNewNode2Parent("bracket", n->t, subroutineDeclaration);
    
    // subroutine body
    parseSubroutineBody(subroutineDeclaration, table);
    
    // a mandatory right brace
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseSubroutineDeclaration", "missing right brace to conclude the method body");
    insertNewNode2Parent("bracket", n->t, subroutineDeclaration);
    
}

void parseParameterList(treeNode* parent, tokenTable* table){
    treeNode* parameterList = insertNewNode2Parent("parameterList", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // if the argument list is empty, next node must be ')'.
    peeknext = peekNextNode(table);
    if(isBracket(')', peeknext)){
        return;
    }
    
    // else, it has at least one argument.
    // a mandatory type
    parseType(parameterList, table);
    
    // an identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseParameterList", "missing identifier for the argument");
    insertNewNode2Parent("identifier", n->t, parameterList);
    
    // zero or more argument
    peeknext = peekNextNode(table);
    while(isSymbol(',', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, parameterList);
        
        parseType(parameterList, table);
        
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseParameterList", "missing identifier for the argument");
        insertNewNode2Parent("identifier", n->t, parameterList);
        
        peeknext = peekNextNode(table);
    }
    
}

void parseSubroutineBody(treeNode* parent, tokenTable* table){
    treeNode* subroutineBody = insertNewNode2Parent("subroutineBody", NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    while(!isBracket('}', peeknext)){
        if(isVariableDeclarationStart(peeknext)){
            parseVariableDeclaration(subroutineBody, table);
            // a semicolon
            n = nextNode(table);
            checkCharValueNodeExpected(n, SEMICOLON, ';', "parseSubroutineBody", "missing semicolon to conclude a variable declaration");
        insertNewNode2Parent("identifier", n->t, subroutineBody);
        }else if(isStatementStart(peeknext)){
            parseStatement(subroutineBody, table);
        }else{
            fprintf(stderr, "Error parseSubroutineBody line %d: unknow contents inside method body\n", peeknext->t->lineNumber);
            exit(1);
        }
    }
}

void parseStatement(treeNode* parent, tokenTable* table){}
void parseIfStatement(treeNode* parent, tokenTable* table){}
void parseSwitchStatement(treeNode* parent, tokenTable* table){}
void parseForStatement(treeNode* parent, tokenTable* table){}
void parseWhileStatement(treeNode* parent, tokenTable* table){}
void parseDoWhileStatement(treeNode* parent, tokenTable* table){}
void parseReturnStatement(treeNode* parent, tokenTable* table){}
void parseContinueStatement(treeNode* parent, tokenTable* table){}
void parseBreakStatement(treeNode* parent, tokenTable* table){}







