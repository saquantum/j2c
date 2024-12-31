#include "parser.h"

CST* parseTokenTable(char* filename, tokenTable* table){
    if(!filename || !table){
        fprintf(stderr, "%sError parseTokenTable: no file name or token table.%s\n", RED, NRM);
        exit(1);
    }
    CST* cst = calloc(1,sizeof(CST));
    if(!cst){
        fprintf(stderr, "%sError parseTokenTable @%s: not enough memory, cannot create parse tree.%s\n", RED, filename, NRM);
        exit(1);
    }
    cst->root = createTreeNode(file_rule, NULL);
    
    //printf("Debug: in statements only mode.\n");
    while(hasNext(table)){
        parseFile(cst->root, table);
    }
    return cst;
}

void parseAnnotation(treeNode* parent, tokenTable* table){
    treeNode* anno = insertNewNode2Parent(annotation_rule, NULL, parent);
    tokenNode* n;
    
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, '@', "parseAnnotation", "missng '@' for the annotation");
    insertNewNode2Parent(at_rule, n->t, anno); 
    
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseAnnotation", "missng annotation type");
    insertNewNode2Parent(identifier_rule, n->t, anno); 
}

void parseTerm(treeNode* parent, tokenTable* table){
    treeNode* term = insertNewNode2Parent(term_rule, NULL, parent);
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

    // use while loop to accomplish chained access: 
    // obj.method1().field1.field2[2].method2()
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
    treeNode* newObject = insertNewNode2Parent(newObject_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // 'new' keyword
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, NEW, "parseNewObject", "missng 'new' keyword to create a new object");
    insertNewNode2Parent(new_rule, n->t, newObject);
    
    // type
    parseType(newObject, table);
    
    // optional generics
    peeknext = peekNextNode(table);
    if(isBracket('<', peeknext)){
        parseGenerics(newObject, table);
    }
    
    // branch: constructor, array or anonymous class
    peeknext = peekNextNode(table);
    if(isBracket('(', peeknext)){
        printf("Debug: new constructor\n");
        // constructor call
        n = nextNode(table);
        insertNewNode2Parent(bracket_rule, n->t, newObject);
        
        parseExpressionList(newObject, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseNewObject", "missng right parenthesis to conclude argument list");
        insertNewNode2Parent(bracket_rule, n->t, newObject);
    }
    else if(isBracket('[', peeknext)){
        // array
        // let's defer the rule, that int[10][] is valid while int[][10] is invalid to semantics.
        printf("Debug: new array\n");
        while(isBracket('[', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(bracket_rule, n->t, newObject);
            
            parseExpression(newObject, table);
            
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, ']', "parseNewObject", "missng right square bracket to conclude array length");
            insertNewNode2Parent(bracket_rule, n->t, newObject);
            peeknext = peekNextNode(table);
        }
        
        // optional initialization
        peeknext = peekNextNode(table);
        if(isBracket('{', peeknext)){
            printf("Debug: new array initialization\n");
            parseArrayInitialization(newObject, table);
        }
    }
    else if(isBracket('{', peeknext)){
        printf("Debug: new anonymous class\n");
        // anonymous class
        // '{'
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, '{', "parseNewObject", "missng left brace to start anonymous class");
        insertNewNode2Parent(compound_rule, n->t, newObject);
        
        parseClassBody(newObject, table);
        
        // '}'
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, '{', "parseNewObject", "missng left brace to start anonymous class");
        insertNewNode2Parent(compound_rule, n->t, newObject);

    }else{
        // the new object expression must either be a constructor call, or an array
        fprintf(stderr, "%sError parseNewObject line %d: missing constructor call or array initialization for a new object%s\n", RED, peeknext->t->lineNumber, NRM);
        exit(1);
    }
}

void parseArrayInitialization(treeNode* parent, tokenTable* table){
    treeNode* arrayInitialization = insertNewNode2Parent(arrayInitialization_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseArrayInitialization", "missng left brace to start array initialization");
    insertNewNode2Parent(compound_rule, n->t, arrayInitialization);
    
    peeknext = peekNextNode(table);
    if(isBracket('{', peeknext)){
        parseArrayInitialization(arrayInitialization, table);
        peeknext = peekNextNode(table);
        while(isSymbol(',', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(comma_rule, n->t, arrayInitialization);
            parseArrayInitialization(arrayInitialization, table);
            peeknext = peekNextNode(table);
        }
    }else{
        parseTerm(arrayInitialization, table);
        peeknext = peekNextNode(table);
        while(isSymbol(',', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(comma_rule, n->t, arrayInitialization);
            parseTerm(arrayInitialization, table);
            peeknext = peekNextNode(table);
        }
    }
    
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseArrayInitialization", "missng right brace to conclude array initialization");
    insertNewNode2Parent(compound_rule, n->t, arrayInitialization);
}

void parseBaseTerm(treeNode* parent, tokenTable* table){
    tokenNode* n;
    tokenNode* peeknext;
    
    if(!parent){
        fprintf(stderr, "%sError parseBaseTerm: null parent node for a base term.%s\n", RED, NRM);
        exit(1);
    }
    peeknext = peekNextNode(table);
    if(!peeknext){
        fprintf(stderr, "%sError parseBaseTerm: unexpected end of tokens.%s\n", RED, NRM);
        exit(1);
    }
    // four terminal cases: true, false, null, this
    if(isKey(BOOL_TRUE, peeknext) || isKey(BOOL_FALSE, peeknext) || isKey(NULLER, peeknext) || isKey(THIS, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(terminalTerm_rule, n->t, parent);
    }
    // token is a number
    else if(isNumber(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(number_rule, n->t, parent);
    }
    // token is a character
    else if(isCharacter(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(character_rule, n->t, parent);
    }
    // token is a string
    else if(isString(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(string_rule, n->t, parent);
    }
    // token is a variable 
    else if(isIdentifier(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(identifier_rule, n->t, parent);
    }
    // term is an expression within a parenthesis
    else if(isBracket('(', peeknext)){
        n = nextNode(table);
        treeNode* parenthesized = insertNewNode2Parent(parenthesizedExpression_rule, NULL, parent);
        insertNewNode2Parent(bracket_rule, n->t, parenthesized);
        
        parseExpression(parenthesized, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseBaseTerm", "missing right parenthesis");
        insertNewNode2Parent(bracket_rule, n->t, parenthesized);
    }
}

void parseFieldAccess(treeNode* parent, tokenTable* table){
    treeNode* field = insertNewNode2Parent(fieldAccess_rule, NULL, parent);
    tokenNode* n;
    
    // dot
    n = nextNode(table);
    insertNewNode2Parent(dot_rule, n->t, field);
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseFieldAccess", "identifier expected after dot");
    insertNewNode2Parent(identifier_rule, n->t, field);
}

void parseArrayAccess(treeNode* parent, tokenTable* table){
    treeNode* array = insertNewNode2Parent(arrayAccess_rule, NULL, parent);
    tokenNode* n;
    
    // left bracket
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '[', "parseArrayAccess", "missing left square bracket to start array access");
    insertNewNode2Parent(bracket_rule, n->t, array);
    
    // expression
    parseExpression(array, table);
    
    // right bracket
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ']', "parseArrayAccess", "missing right square bracket to conclude array access");
    insertNewNode2Parent(bracket_rule, n->t, array);
}

void parseSubroutineCall(treeNode* parent, tokenTable* table){
    treeNode* call = insertNewNode2Parent(subroutineCall_rule, NULL, parent);
    tokenNode* n;

    // left parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseSubroutineCall", "missing left parenthesis to start subroutine call");
    insertNewNode2Parent(bracket_rule, n->t, call);
    
    printf("Debug: subroutine call at line %d\n", n->t->lineNumber);
    
    // argument list
    parseExpressionList(call, table);
    
    // right parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseSubroutineCall", "missing right parenthesis to conclude subroutine call");
    insertNewNode2Parent(bracket_rule, n->t, call);
}

void parseExpressionList(treeNode* parent, tokenTable* table){
    treeNode* list = insertNewNode2Parent(expressionList_rule, NULL, parent);
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
        insertNewNode2Parent(comma_rule, n->t, list);
        
        parseExpression(list, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseExpression(treeNode* parent, tokenTable* table){
    treeNode* exp = insertNewNode2Parent(expression_rule, NULL, parent);
    
    parseTernaryExpression(exp, table);
}

void parseTernaryExpression(treeNode* parent, tokenTable* table){
    treeNode* ternary = insertNewNode2Parent(ternaryExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseLogicalOrExpression(ternary, table);
    
    peeknext = peekNextNode(table);
    if(!isSymbol('?', peeknext)){
        return; // no ternary expression, this reduces to a simple expression.
    }
    
    // '?'
    n = nextNode(table);
    insertNewNode2Parent(ternaryOperator_rule, n->t, ternary);
    
    parseExpression(ternary, table);
    
    // ':'   
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, ':', "parseTernaryExpression", "missing colon in ternary expression");
    insertNewNode2Parent(ternaryOperator_rule, n->t, ternary);
    
    parseExpression(ternary, table);
}

void parseLogicalOrExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalOr = insertNewNode2Parent(logicalOrExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseLogicalAndExpression(logicalOr, table);
    
    peeknext = peekNextNode(table);
    while(isOperator("||", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, logicalOr);
        printf("Debug: logical operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
        parseLogicalAndExpression(logicalOr, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseLogicalAndExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalAnd = insertNewNode2Parent(logicalAndExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseBitwiseOrExpression(logicalAnd, table);
    
    peeknext = peekNextNode(table);
    while(isOperator("&&", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, logicalAnd);
        printf("Debug: logical operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
        parseBitwiseOrExpression(logicalAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseBitwiseOrExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseOr = insertNewNode2Parent(bitwiseOrExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseBitwiseXorExpression(bitwiseOr, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('|', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, bitwiseOr);
        printf("Debug: bitwise operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        parseBitwiseXorExpression(bitwiseOr, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseBitwiseXorExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseXor = insertNewNode2Parent(bitwiseXorExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseBitwiseAndExpression(bitwiseXor, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('^', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, bitwiseXor);
        printf("Debug: bitwise operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        parseBitwiseAndExpression(bitwiseXor, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseBitwiseAndExpression(treeNode* parent, tokenTable* table){
    treeNode* bitwiseAnd = insertNewNode2Parent(bitwiseAndExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseEqualityExpression(bitwiseAnd, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('&', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, bitwiseAnd);
        printf("Debug: bitwise operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        parseEqualityExpression(bitwiseAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseEqualityExpression(treeNode* parent, tokenTable* table){
    treeNode* equality = insertNewNode2Parent(equalityExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseRelationalExpression(equality, table);
    
    peeknext = peekNextNode(table);
    while(isOperator("==", peeknext) || isOperator("!=", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, equality);
        printf("Debug: equality operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
        parseRelationalExpression(equality, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseRelationalExpression(treeNode* parent, tokenTable* table) {
    treeNode* relational = insertNewNode2Parent(relationalExpression_rule, NULL, parent);
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
        tokenNode* peeknextnext = peeknextnext = peeknext ? peeknext->next : NULL;
        if (isSymbol('=', peeknextnext)) {
            // Combine '<=' or '>='
            char combined[3] = {peeknext->t->data.char_val, peeknextnext->t->data.char_val, 0};
            peeknext->t->type = OPERATOR;
            peeknext->t->data.str_val = mystrdup(combined);
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
        insertNewNode2Parent(operator_rule, n->t, relational);
        if (isSymbol('<', n) || isSymbol('>', n)) {
            printf("Debug: relational operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        }else{
            printf("Debug: relational operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
        }
        parseShiftExpression(relational, table);
    }        
    else if (isKey(INSTANCEOF, peeknext)) {
    // Handle instanceof keyword
    tokenNode* n = nextNode(table);
    insertNewNode2Parent(instanceof_rule, n->t, relational);
    parseReferenceType(relational, table);
    }
}


void parseShiftExpression(treeNode* parent, tokenTable* table){
    treeNode* shift = insertNewNode2Parent(shiftExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseAdditiveExpression(shift, table);
    
    peeknext = peekNextNode(table);
    tokenNode* peeknextnext = peeknext ? peeknext->next : NULL;
    
    while( ( isSymbol('>', peeknext) || isSymbol('<', peeknext) ) && isSymbol(peeknext->t->data.char_val, peeknextnext) ){
        // modify the token table, combine the two nodes into one
        char combined[3] = {peeknext->t->data.char_val, peeknextnext->t->data.char_val, 0};
        peeknext->t->type = OPERATOR;
        peeknext->t->data.str_val = mystrdup(combined);
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
        insertNewNode2Parent(operator_rule, n->t, shift);
        
        if (isSymbol('<', n) || isSymbol('>', n)) {
            printf("Debug: shift operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        }else{
            printf("Debug: shift operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
        }
        
        parseAdditiveExpression(shift, table);
        
        peeknext = peekNextNode(table);
        peeknextnext = peeknext ? peeknext->next : NULL;
    } 
}

void parseAdditiveExpression(treeNode* parent, tokenTable* table){
    treeNode* additive = insertNewNode2Parent(additiveExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseMultiplicativeExpression(additive, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('+', peeknext) || isSymbol('-', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, additive);
        printf("Debug: additive operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        parseMultiplicativeExpression(additive, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseMultiplicativeExpression(treeNode* parent, tokenTable* table){
    treeNode* multiplicative = insertNewNode2Parent(multiplicativeExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseCastExpression(multiplicative, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol('*', peeknext) || isSymbol('/', peeknext) || isSymbol('%', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, multiplicative);
        printf("Debug: multiplicative operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        parseCastExpression(multiplicative, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseCastExpression(treeNode* parent, tokenTable* table){
    treeNode* cast = insertNewNode2Parent(castExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isBracket('(', peeknext) && isPotentialCasting(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(bracket_rule, n->t, cast);
        
        parseType(cast, table);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ')', "parseCastExpression", "missing right parenthesis to conclude casting");
        insertNewNode2Parent(bracket_rule, n->t, cast);
        printf("Debug: casting at line %d\n", n->t->lineNumber);
        parseCastExpression(cast, table);
    }else{
        parseUnaryExpression(cast, table);
    }
}

void parseUnaryExpression(treeNode* parent, tokenTable* table){
    treeNode* unary = insertNewNode2Parent(unaryExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isSymbol('!', peeknext) || isSymbol('-', peeknext) || isSymbol('~', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, unary);
        printf("Debug: unary operator %c at line %d\n", n->t->data.char_val, n->t->lineNumber);
        parseUnaryExpression(unary, table);

        return;
    }
    else if(isOperator("++", peeknext) || isOperator("--", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, unary);
        printf("Debug: prefix operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
        parseTerm(unary, table);

        return;
    }
    else{
        parsePostfixExpression(unary, table);
    }
}

void parsePostfixExpression(treeNode* parent, tokenTable* table){
    treeNode* postfix = insertNewNode2Parent(postfixExpression_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    parseTerm(postfix, table);
    
    peeknext = peekNextNode(table);
    if(isOperator("++", peeknext) || isOperator("--", peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(operator_rule, n->t, postfix);
        printf("Debug: postfix operator %s at line %d\n", n->t->data.str_val, n->t->lineNumber);
    }
}

void parseType(treeNode* parent, tokenTable* table){
    treeNode* type = insertNewNode2Parent(type_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(isKey(CHAR, peeknext) || isKey(INT, peeknext) || isKey(BOOLEAN, peeknext) || isKey(LONG, peeknext) || isKey(DOUBLE, peeknext)){
        
        n = nextNode(table);
        insertNewNode2Parent(primitiveType_rule, n->t, type);
        
        return;
    }
    printf("Debug: the type is a reference type.\n");
    parseReferenceType(type, table);
    printf("Debug: end reference type.\n");
    
}

void parseReferenceType(treeNode* parent, tokenTable* table){
    treeNode* reference = insertNewNode2Parent(referenceType_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    n = nextNode(table);
    if(!n){
        fprintf(stderr, "%sError parseReferenceType: unexpected end of tokens%s\n", RED, NRM);
        exit(1);
    }
    printf("Debug: next token in reference type:\n");
    printCurrentToken(n);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseReferenceType", "missing identifier for reference type");
    insertNewNode2Parent(identifier_rule, n->t, reference);
    
    peeknext = peekNextNode(table);
    if(isSymbol('<', peeknext)){
        parseGenerics(reference, table);
    }
}

void parseGenerics(treeNode* parent, tokenTable* table){
    treeNode* generics = insertNewNode2Parent(generics_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    //printf("Debug: begin parsing generics\n");
    
    // use depth to mimic stack
    int depth = 0;
    
    // '<'
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, '<', "parseGenerics", "missing '<' to start generics");
    insertNewNode2Parent(langle_rule, n->t, generics);
    depth++;
    
    while(depth > 0){
        peeknext = peekNextNode(table);
        if(!peeknext){
            fprintf(stderr, "%sError parseGenerics: unexpected end of tokens inside generics%s\n", RED, NRM);
            exit(1);
        }
        
        if(peeknext->t->type==SYMBOL && peeknext->t->data.char_val!='?'){
            if(peeknext->t->data.char_val=='<'){
                n = nextNode(table);
                insertNewNode2Parent(langle_rule, n->t, generics);
                depth++;
            }else if(peeknext->t->data.char_val=='>'){
                n = nextNode(table);
                insertNewNode2Parent(rangle_rule, n->t, generics);
                depth--;
            }else if(peeknext->t->data.char_val==','){
                n = nextNode(table);
                insertNewNode2Parent(comma_rule, n->t, generics);
            }else{
                fprintf(stderr, "%sError parseGenerics: unexpected symbol %c in generics%s\n", RED, peeknext->t->data.char_val, NRM);
                exit(1);
            }
        }else if(isIdentifier(peeknext) || isSymbol('?', peeknext)){
            parseTypeArgument(generics, table);
        }else {
            fprintf(stderr, "%sError parseGenerics: unexpected token or end of tokens in generics%s\n", RED, NRM);
            exit(1);
        }
        
    }
    //printf("Debug: end parsing generics\n");
    //printTokenTable(table);
    
}

void parseTypeArgument(treeNode* parent, tokenTable* table){
    treeNode* typeArgument = insertNewNode2Parent(typeArgument_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    if(!peeknext){
        fprintf(stderr, "%sError parseTypeArgument: unexpected end of tokens%s\n", RED, NRM);
        exit(1);
    }
    
    if(isSymbol('?', peeknext) || isKey(EXTENDS, peeknext->next) || isKey(SUPER, peeknext->next)){
        // '?' or identifier
        if(isSymbol('?', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(wildcard_rule, n->t, typeArgument); 
        }else{
            n = nextNode(table);
            checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseTypeArgument", "missing identifier for type argument in generics");
            insertNewNode2Parent(identifier_rule, n->t, typeArgument); 
        }
        
        peeknext = peekNextNode(table);
        if(isKey(EXTENDS, peeknext) || isKey(SUPER, peeknext)){
            // extends or super
            n = nextNode(table);
            if(isKey(EXTENDS, peeknext)){
                insertNewNode2Parent(extends_rule, n->t, typeArgument);
            }else if(isKey(SUPER, peeknext)){
                insertNewNode2Parent(super_rule, n->t, typeArgument);
            }
            
            parseReferenceType(typeArgument, table);
        }
    }
    else if(isIdentifier(peeknext)){
        parseReferenceType(typeArgument, table);
    }else{
        fprintf(stderr, "%sError parseTypeArgument: invalid token in type arguments%s\n", RED, NRM);
        exit(1);
    }
}

void parseAssignment(treeNode* parent, tokenTable* table){
    treeNode* assignment = insertNewNode2Parent(assignment_rule, NULL, parent);
    tokenNode* n;
    
    printf("Debug: assignment at line %d\n", peekNextNode(table)->t->lineNumber);
    
    // a lvalue (term)
    parseTerm(assignment, table);
    
    // mandatory assignment operator
    n = nextNode(table);
    if(!(isSymbol('=', n) || isOperator("+=", n) || isOperator("-=", n) || isOperator("*=", n) || isOperator("/=", n))){
        fprintf(stderr, "%sError parseAssignment line %d: missing assignment operator%s\n", RED, n->t->lineNumber, NRM);
        exit(1);
    }
    insertNewNode2Parent(assignmentOperator_rule, n->t, assignment); 
    
    // an expression
    
    parseExpression(assignment, table);
}

void parseVariableDeclaration(treeNode* parent, tokenTable* table){
    treeNode* variableDeclaration = insertNewNode2Parent(variableDeclaration_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // an optional access modifier
    peeknext = peekNextNode(table);
    if(isKey(PUBLIC, peeknext) || isKey(PRIVATE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(accessModifier_rule, n->t, variableDeclaration); 
    }

    // zero or more non-access modifier
    peeknext = peekNextNode(table);
    while(isKey(STATIC, peeknext) || isKey(FINAL, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(nonAccessModifier_rule, n->t, variableDeclaration);
        peeknext = peekNextNode(table);
    }
    
    // a mandatory type
    parseType(variableDeclaration, table);
    
    // an optional array definition
    peeknext = peekNextNode(table);
    while(isBracket('[', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(bracket_rule, n->t, variableDeclaration);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ']', "parseVariableDeclaration", "missing right square bracket to conclude array definition");
        insertNewNode2Parent(bracket_rule, n->t, variableDeclaration);
        
        peeknext = peekNextNode(table);
    }
    
    // an identifier, or an assignment
    peeknext = peekNextNode(table);
    if(isPotentialAssignment(peeknext)){
        printf("Debug: assignment in variable declaration at line %d\n", peeknext->t->lineNumber);
        parseAssignment(variableDeclaration, table);
    }else{
        printf("Debug: uninitialized variable declaration at line %d\n", peeknext->t->lineNumber);
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseVariableDeclaration", "missing identifier for the variable");
        insertNewNode2Parent(identifier_rule, n->t, variableDeclaration);
    }
    
    // zero or more definitions in the same line separated by comma
    peeknext = peekNextNode(table);
    while(isSymbol(',', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(comma_rule, n->t, variableDeclaration);
        peeknext = peekNextNode(table);
        if(isPotentialAssignment(peeknext)){
            printf("Debug: assignment in variable declaration at line %d\n", peeknext->t->lineNumber);
            parseAssignment(variableDeclaration, table);
        }else{
            printf("Debug: uninitialized variable declaration at line %d\n", peeknext->t->lineNumber);
            n = nextNode(table);
            checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseVariableDeclaration", "missing identifier for the variable");
            insertNewNode2Parent(identifier_rule, n->t, variableDeclaration);
        }
        peeknext = peekNextNode(table);
    }
}

void parseSubroutineDeclaration(treeNode* parent, tokenTable* table){
    treeNode* subroutineDeclaration = insertNewNode2Parent(subroutineDeclaration_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    printf("Debug: subroutine declaration start at line %d\n", peekNextNode(table)->t->lineNumber);
    
    // optional annotation
    peeknext = peekNextNode(table);
    if(isSymbol('@', peeknext)){
        parseAnnotation(subroutineDeclaration, table);
    }
    
    // optional access modifier
    peeknext = peekNextNode(table);
    if(isKey(PUBLIC, peeknext) || isKey(PRIVATE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(accessModifier_rule, n->t, subroutineDeclaration); 
    }
    
    // zero or more non-access modifier
    peeknext = peekNextNode(table);
    while(isKey(STATIC, peeknext) || isKey(FINAL, peeknext) || isKey(ABSTRACT, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(nonAccessModifier_rule, n->t, subroutineDeclaration);
        peeknext = peekNextNode(table);
    }
    
    // optional native
    peeknext = peekNextNode(table);
    if(isKey(NATIVE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(native_rule, n->t, subroutineDeclaration); 
    }
    
    // optional type bounds
    peeknext = peekNextNode(table);
    if(isSymbol('<', peeknext)){
        parseTypeBoundList(subroutineDeclaration, table);
    }
    
    // if next token is identifier and next next token is '(', it can only be constructor
    peeknext = peekNextNode(table);
    if(!(isIdentifier(peeknext) && isBracket('(', peeknext->next))){
    
        // else, is has a return type or void
        peeknext = peekNextNode(table);
        if(isKey(VOID, peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(type_rule, n->t, subroutineDeclaration);
        }else{
            parseType(subroutineDeclaration, table);
        }
    
    }
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseSubroutineDeclaration", "missing identifier for the method");
    insertNewNode2Parent(identifier_rule, n->t, subroutineDeclaration);
    
    // mandatory left parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseSubroutineDeclaration", "missing left parenthesis to start the argument list");
    insertNewNode2Parent(bracket_rule, n->t, subroutineDeclaration);
    
    // parameter list
    parseParameterList(subroutineDeclaration, table);
    
    // mandatory right parenthesis
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseSubroutineDeclaration", "missing right parenthesis to conclude the argument list");
    insertNewNode2Parent(bracket_rule, n->t, subroutineDeclaration);
    
    // mandatory left brace
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseSubroutineDeclaration", "missing left brace to start the method body");
    insertNewNode2Parent(compound_rule, n->t, subroutineDeclaration);
    
    // subroutine body
    parseSubroutineBody(subroutineDeclaration, table);
    
    // mandatory right brace
    n = nextNode(table);
    printf("!!!!!Debug: subroutine declaration end at line %d\n", n->t->lineNumber);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseSubroutineDeclaration", "missing right brace to conclude the method body");
    insertNewNode2Parent(compound_rule, n->t, subroutineDeclaration);
    
    
}

void parseParameterList(treeNode* parent, tokenTable* table){
    treeNode* parameterList = insertNewNode2Parent(parameterList_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // if the argument list is empty, next node must be ')'.
    peeknext = peekNextNode(table);
    if(isBracket(')', peeknext)){
        return;
    }
    
    // else, it has at least one argument.
    // optional final keyword
    peeknext = peekNextNode(table);
    if(isKey(FINAL, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(const_rule, n->t, parameterList);
    }
    
    // a mandatory type
    parseType(parameterList, table);
    
    // optional array square brackets
    peeknext = peekNextNode(table);
    while(isBracket('[', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(bracket_rule, n->t, parameterList);
        
        n = nextNode(table);
        checkCharValueNodeExpected(n, BRACKET, ']', "parseParameterList", "missing right square bracket for the formal array");
        insertNewNode2Parent(bracket_rule, n->t, parameterList);
        peeknext = peekNextNode(table);
    }
    
    // an identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseParameterList", "missing identifier for the argument");
    insertNewNode2Parent(identifier_rule, n->t, parameterList);
    
    // zero or more argument
    peeknext = peekNextNode(table);
    while(isSymbol(',', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(comma_rule, n->t, parameterList);
        
        // optional final keyword
        peeknext = peekNextNode(table);
        if(isKey(FINAL, peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(const_rule, n->t, parameterList);
        }
        
        parseType(parameterList, table);
        
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseParameterList", "missing identifier for the argument");
        insertNewNode2Parent(identifier_rule, n->t, parameterList);
        
        peeknext = peekNextNode(table);
    }
    
}

void parseTypeBoundList(treeNode* parent, tokenTable* table){
    treeNode* typeBoundList = insertNewNode2Parent(typeBoundList_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, '<', "parseTypeBoundList", "missing left angle to start the parameter bound list");
    insertNewNode2Parent(langle_rule, n->t, typeBoundList);
    
    parseTypeBound(typeBoundList, table);
    
    peeknext = peekNextNode(table);
    while(isSymbol(',', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(comma_rule, n->t, typeBoundList);
        
        parseTypeBound(typeBoundList, table);
        
        peeknext = peekNextNode(table);
    }
    
    n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, '>', "parseTypeBoundList", "missing right angle to conclude the parameter bound list");
    insertNewNode2Parent(rangle_rule, n->t, typeBoundList);
}

void parseTypeBound(treeNode* parent, tokenTable* table){
    treeNode* typeBound = insertNewNode2Parent(typeBound_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseTypeBound", "missing identifier for the type to be bounded");
    insertNewNode2Parent(identifier_rule, n->t, typeBound);
    
    // optional boundedness
    peeknext = peekNextNode(table);
    if(isKey(EXTENDS, peeknext)){
        parseConstraint(typeBound, table);
    }
}

void parseConstraint(treeNode* parent, tokenTable* table){
    treeNode* constraint = insertNewNode2Parent(constraint_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // extends one class or interface
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, EXTENDS, "parseConstraint", "missing extends keyword for a constraint");
    insertNewNode2Parent(extends_rule, n->t, constraint);
    
    parseType(constraint, table);
    
    // optionally extends some more interfaces
    peeknext = peekNextNode(table);
    while(isSymbol('&', peeknext)){
    
        n = nextNode(table);
        insertNewNode2Parent(moreInterface_rule, n->t, constraint);
        
        parseType(constraint, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseSubroutineBody(treeNode* parent, tokenTable* table){
    treeNode* subroutineBody = insertNewNode2Parent(subroutineBody_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    while(!isBracket('}', peeknext)){
        if(isVariableDeclarationStart(peeknext)){
            printf("Debug: variable declaration in subroutine body at line %d\n", peeknext->t->lineNumber);
            parseVariableDeclaration(subroutineBody, table);
            // a semicolon
            n = nextNode(table);
            checkCharValueNodeExpected(n, SEMICOLON, ';', "parseSubroutineBody", "missing semicolon to conclude a variable declaration");
            insertNewNode2Parent(semicolon_rule, n->t, subroutineBody);
            peeknext = peekNextNode(table);
        }else if(isStatementStart(peeknext)){
            printf("Debug: statement in subroutine body at line %d\n", peeknext->t->lineNumber);
            parseStatement(subroutineBody, table);
            peeknext = peekNextNode(table);
        }else{
            fprintf(stderr, "%sError parseSubroutineBody line %d: unknow contents inside method body%s\n", RED, peeknext->t->lineNumber, NRM);
            exit(1);
        }
    }
}

void parseStatement(treeNode* parent, tokenTable* table){
    treeNode* statement = insertNewNode2Parent(statement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    printf("Debug: statement at line %d\n", peeknext->t->lineNumber);
    if(isKey(IF, peeknext)){
        parseIfStatement(statement, table);
    }else if(isKey(SWITCH, peeknext)){
        parseSwitchStatement(statement, table);
    }else if(isKey(FOR, peeknext)){
        parseForStatement(statement, table);
    }else if(isKey(WHILE, peeknext)){
        parseWhileStatement(statement, table);
    }else if(isKey(DO, peeknext)){
        parseDoWhileStatement(statement, table);
    }else if(isKey(RETURN, peeknext)){
        parseReturnStatement(statement, table);
    }else if(isKey(BREAK, peeknext)){
        parseBreakStatement(statement, table);
    }else if(isKey(CONTINUE, peeknext)){
        parseContinueStatement(statement, table);
    }else if(isKey(STATIC, peeknext)){
        parseStaticStatement(statement, table);
    }else if(isBracket('{', peeknext)){
        parseCodeBlock(statement, table);
    }else if(isSemicolon(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(semicolon_rule, n->t, statement);
    }else if(isVariableDeclarationStart(peeknext)){
        parseVariableDeclaration(statement, table);
        n = nextNode(table);
        checkCharValueNodeExpected(n, SEMICOLON, ';', "parseStatement", "missing semicolon to conclude a variable declaration");
        insertNewNode2Parent(semicolon_rule, n->t, statement);
    }else if(isPotentialAssignment(peeknext)){
        parseAssignment(statement, table);
        n = nextNode(table);
        checkCharValueNodeExpected(n, SEMICOLON, ';', "parseStatement", "missing semicolon to conclude an assignment");
        insertNewNode2Parent(semicolon_rule, n->t, statement);
    }else if(isExpressionStart(peeknext)){
        printf("Debug: parse expression at line %d\n", peeknext->t->lineNumber);
        parseExpression(statement, table);
        n = nextNode(table);
        checkCharValueNodeExpected(n, SEMICOLON, ';', "parseStatement", "missing semicolon to conclude an expression");
        insertNewNode2Parent(semicolon_rule, n->t, statement);
    }else{
        fprintf(stderr, "%sError parseStatement line %d: invalid statement%s\n", RED, peeknext->t->lineNumber, NRM);
        exit(1);
    }
}

void parseIfStatement(treeNode* parent, tokenTable* table){
    treeNode* ifStatement = insertNewNode2Parent(ifStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // if
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, IF, "parseIfStatement", "missing 'if' keyword for an if statement");
    insertNewNode2Parent(if_rule, n->t, ifStatement);
    
    // '('
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseIfStatement", "missing left parenthesis to start the if condition");
    insertNewNode2Parent(bracket_rule, n->t, ifStatement);
    
    parseExpression(ifStatement, table);
    
    // ')'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseIfStatement", "missing right parenthesis to conlcude the if condition");
    insertNewNode2Parent(bracket_rule, n->t, ifStatement);
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseIfStatement", "missing left brace to start the if compound");
    insertNewNode2Parent(compound_rule, n->t, ifStatement);
    
    // optional body
    peeknext = peekNextNode(table);
    if(!isBracket('}', peeknext)){
        parseStatement(ifStatement, table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseIfStatement", "missing right brace to conclude the if compound");
    insertNewNode2Parent(compound_rule, n->t, ifStatement);
    
    // else or else-if?
    peeknext = peekNextNode(table);
    while(isKey(ELSE, peeknext)){
        if(isKey(IF, peeknext->next)){
            // else-if 
            // else
            n = nextNode(table);
            insertNewNode2Parent(else_rule, n->t, ifStatement);
            
            // if
            n = nextNode(table);
            insertNewNode2Parent(if_rule, n->t, ifStatement);
    
            // '('
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, '(', "parseIfStatement", "missing left parenthesis to start the if condition");
            insertNewNode2Parent(bracket_rule, n->t, ifStatement);
    
            parseExpression(ifStatement, table);
    
            // ')'
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, ')', "parseIfStatement", "missing right parenthesis to conlcude the if condition");
            insertNewNode2Parent(bracket_rule, n->t, ifStatement);
    
            // '{'
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, '{', "parseIfStatement", "missing left brace to start the if compound");
            insertNewNode2Parent(compound_rule, n->t, ifStatement);
    
            // optional body
            peeknext = peekNextNode(table);
            if(!isBracket('}', peeknext)){
                parseStatement(ifStatement, table);
            }
    
            // '}'
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, '}', "parseIfStatement", "missing right brace to conclude the if compound");
            insertNewNode2Parent(compound_rule, n->t, ifStatement);
            
        }else{
            // else
            n = nextNode(table);
            insertNewNode2Parent(else_rule, n->t, ifStatement);
            
            // '{'
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, '{', "parseIfStatement", "missing left brace to start the if compound");
            insertNewNode2Parent(compound_rule, n->t, ifStatement);
            
            // optional body
            peeknext = peekNextNode(table);
            if(!isBracket('}', peeknext)){
                parseStatement(ifStatement, table);
            }
            
            // '}'
            n = nextNode(table);
            checkCharValueNodeExpected(n, BRACKET, '}', "parseIfStatement", "missing right brace to conclude the if compound");
            insertNewNode2Parent(compound_rule, n->t, ifStatement);
            return;
        }
        peeknext = peekNextNode(table);
    }
}

void parseSwitchStatement(treeNode* parent, tokenTable* table){
    treeNode* switchStatement = insertNewNode2Parent(switchStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // switch
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, SWITCH, "parseSwitchStatement", "missing 'switch' keyword for a switch statement");
    insertNewNode2Parent(switch_rule, n->t, switchStatement);
    
    // '('
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseSwitchStatement", "missing left parenthesis for a switch statement");
    insertNewNode2Parent(bracket_rule, n->t, switchStatement);
      
    // mandatory expression
    parseExpression(switchStatement, table);

    // ')'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseSwitchStatement", "missing right parenthesis for a switch statement");
    insertNewNode2Parent(bracket_rule, n->t, switchStatement);
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseSwitchStatement", "missing left brace to start a switch compound");
    insertNewNode2Parent(compound_rule, n->t, switchStatement); 
    
    peeknext = peekNextNode(table);
    while(isKey(CASE, peeknext) || isKey(DEFAULT, peeknext)){
        if(isKey(CASE, peeknext)){
            // case
            n = nextNode(table);
            insertNewNode2Parent(case_rule, n->t, switchStatement); 
        
            parseExpression(switchStatement, table);
        
            // ':'
            n = nextNode(table);
            checkCharValueNodeExpected(n, SYMBOL, ':', "parseSwitchStatement", "missing colon after case");
            insertNewNode2Parent(colon_rule, n->t, switchStatement); 
        
            peeknext = peekNextNode(table);
            while(isStatementStart(peeknext)){
                parseStatement(switchStatement, table);
                peeknext = peekNextNode(table);
            }
        }else if(isKey(DEFAULT, peeknext)){
        // default
            n = nextNode(table);
            insertNewNode2Parent(default_rule, n->t, switchStatement); 
        
            // ':'
            n = nextNode(table);
            checkCharValueNodeExpected(n, SYMBOL, ':', "parseSwitchStatement", "missing colon after default");
            insertNewNode2Parent(colon_rule, n->t, switchStatement); 
        
            peeknext = peekNextNode(table);
            while(isStatementStart(peeknext)){
                parseStatement(switchStatement, table);
                peeknext = peekNextNode(table);
            }
        }
        peeknext = peekNextNode(table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseSwitchStatement", "missing right brace to conclude a switch compound");
    insertNewNode2Parent(compound_rule, n->t, switchStatement); 
}

void parseForStatement(treeNode* parent, tokenTable* table){
    treeNode* forStatement = insertNewNode2Parent(forStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // for
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, FOR, "parseForStatement", "missing 'for' keyword for a for statement");
    insertNewNode2Parent(for_rule, n->t, forStatement);
    printf("Debug: for statement at line %d\n", n->t->lineNumber);
    // '('
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseForStatement", "missing left parenthesis for a for statement");
    insertNewNode2Parent(bracket_rule, n->t, forStatement);
    
    // optional variable declaration or assignment 
    peeknext = peekNextNode(table);
    if(isVariableDeclarationStart(peeknext)){
        printf("Debug: varDec before first semicolon\n");
        parseVariableDeclaration(forStatement, table);
    }
    else if(isPotentialAssignment(peeknext)){
        printf("Debug: assignment before first semicolon\n");
        parseAssignment(forStatement, table);
    }
    
    // ';'
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseForStatement", "missing semicolon");
    insertNewNode2Parent(semicolon_rule, n->t, forStatement);
    
    // optional expression
    peeknext = peekNextNode(table);
    if(isExpressionStart(peeknext)){
        parseExpression(forStatement, table);
    }
    
    // ';'
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseForStatement", "missing semicolon");
    insertNewNode2Parent(semicolon_rule, n->t, forStatement);
    
    // optional expression or assignment
    peeknext = peekNextNode(table);
    if(isPotentialAssignment(peeknext)){
        parseAssignment(forStatement, table);
    }else if(isExpressionStart(peeknext)){
        parseExpression(forStatement, table);
    }
    
    // ')'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseForStatement", "missing right parenthesis for a for statement");
    insertNewNode2Parent(bracket_rule, n->t, forStatement);
    
    // end with a semicolon or a compound
    peeknext = peekNextNode(table);
    if(isSemicolon(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(semicolon_rule, n->t, forStatement);
        return;
    }
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseForStatement", "missing left brace to start a for compound");
    insertNewNode2Parent(compound_rule, n->t, forStatement); 
    
    peeknext = peekNextNode(table);
    while(isStatementStart(peeknext)){
        parseStatement(forStatement, table);
        peeknext = peekNextNode(table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseForStatement", "missing right brace to conclude a for compound");
    insertNewNode2Parent(compound_rule, n->t, forStatement); 
}

void parseWhileStatement(treeNode* parent, tokenTable* table){
    treeNode* whileStatement = insertNewNode2Parent(whileStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // while
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, WHILE, "parseWhileStatement", "missing 'while' keyword for a while statement");
    insertNewNode2Parent(while_rule, n->t, whileStatement);
    
    // '('
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseWhileStatement", "missing left parenthesis for a while statement");
    insertNewNode2Parent(bracket_rule, n->t, whileStatement);
      
    // mandatory expression
    parseExpression(whileStatement, table);

    // ')'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseWhileStatement", "missing right parenthesis for a while statement");
    insertNewNode2Parent(bracket_rule, n->t, whileStatement);
    
    // end with a semicolon or a compound
    peeknext = peekNextNode(table);
    if(isSemicolon(peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(semicolon_rule, n->t, whileStatement);
        return;
    }
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseWhileStatement", "missing left brace to start a while compound");
    insertNewNode2Parent(compound_rule, n->t, whileStatement); 
    
    peeknext = peekNextNode(table);
    while(isStatementStart(peeknext)){
        parseStatement(whileStatement, table);
        peeknext = peekNextNode(table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseWhileStatement", "missing right brace to conclude a while compound");
    insertNewNode2Parent(compound_rule, n->t, whileStatement); 
}

void parseDoWhileStatement(treeNode* parent, tokenTable* table){
    treeNode* doWhileStatement = insertNewNode2Parent(doWhileStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // do
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, DO, "parseDoWhileStatement", "missing 'do' keyword for a do-while statement");
    insertNewNode2Parent(do_rule, n->t, doWhileStatement);
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseDoWhileStatement", "missing left brace to start a do-while compound");
    insertNewNode2Parent(compound_rule, n->t, doWhileStatement); 
    
    peeknext = peekNextNode(table);
    while(isStatementStart(peeknext)){
        parseStatement(doWhileStatement, table);
        peeknext = peekNextNode(table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseDoWhileStatement", "missing right brace to conclude a do-while compound");
    insertNewNode2Parent(compound_rule, n->t, doWhileStatement); 
    
    // while
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, WHILE, "parseDoWhileStatement", "missing 'while' keyword for a do-while statement");
    insertNewNode2Parent(while_rule, n->t, doWhileStatement);
    
    // '('
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '(', "parseDoWhileStatement", "missing left parenthesis for a do-while statement");
    insertNewNode2Parent(bracket_rule, n->t, doWhileStatement);
      
    // mandatory expression
    parseExpression(doWhileStatement, table);

    // ')'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, ')', "parseDoWhileStatement", "missing right parenthesis for a do-while statement");
    insertNewNode2Parent(bracket_rule, n->t, doWhileStatement);
    
    // must end with a semicolon
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseDoWhileStatement", "missing semicolon to conclude a do-while statement");
    insertNewNode2Parent(semicolon_rule, n->t, doWhileStatement);
       
}

void parseReturnStatement(treeNode* parent, tokenTable* table){
    treeNode* returnStatement = insertNewNode2Parent(returnStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // return
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, RETURN, "parseReturnStatement", "missing 'return' keyword for a return statement");
    insertNewNode2Parent(return_rule, n->t, returnStatement);
    
    // optional expression
    peeknext = peekNextNode(table);
    if(!isSemicolon(peeknext)){
        parseExpression(returnStatement, table);
    }
    
    // semicolon
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseReturnStatement", "missing semicolon to conclude a return statement");
    insertNewNode2Parent(semicolon_rule, n->t, returnStatement);
    
}

void parseContinueStatement(treeNode* parent, tokenTable* table){
    treeNode* continueStatement = insertNewNode2Parent(continueStatement_rule, NULL, parent);
    tokenNode* n;
    
    // continue
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, CONTINUE, "parseContinueStatement", "missing 'continue' keyword for a return statement");
    insertNewNode2Parent(continue_rule, n->t, continueStatement);
    
    // semicolon
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseReturnStatement", "missing semicolon to conclude a continue statement");
    insertNewNode2Parent(semicolon_rule, n->t, continueStatement);    
}

void parseBreakStatement(treeNode* parent, tokenTable* table){
    treeNode* breakStatement = insertNewNode2Parent(breakStatement_rule, NULL, parent);
    tokenNode* n;
    
    // break
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, BREAK, "parseBreakStatement", "missing 'break' keyword for a return statement");
    insertNewNode2Parent(break_rule, n->t, breakStatement);
    
    // semicolon
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseBreakStatement", "missing semicolon to conclude a break statement");
    insertNewNode2Parent(semicolon_rule, n->t, breakStatement); 
}

void parseStaticStatement(treeNode* parent, tokenTable* table){
    treeNode* staticStatement = insertNewNode2Parent(staticStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // static
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, STATIC, "parseStaticStatement", "missing 'static' keyword for a return statement");
    insertNewNode2Parent(static_rule, n->t, staticStatement);
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseStaticStatement", "missing left brace to start a static statement");
    insertNewNode2Parent(compound_rule, n->t, staticStatement); 
    
    peeknext = peekNextNode(table);
    while(isStatementStart(peeknext)){
        parseStatement(staticStatement, table);
        peeknext = peekNextNode(table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseStaticStatement", "missing right brace to conclude a static statement");
    insertNewNode2Parent(compound_rule, n->t, staticStatement); 
}

void parseCodeBlock(treeNode* parent, tokenTable* table){
    treeNode* codeBlock = insertNewNode2Parent(codeBlock_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseCodeBlock", "missing left brace to start a code block");
    insertNewNode2Parent(compound_rule, n->t, codeBlock); 
    
    peeknext = peekNextNode(table);
    while(isStatementStart(peeknext)){
        parseStatement(codeBlock, table);
        peeknext = peekNextNode(table);
    }
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseCodeBlock", "missing right brace to conclude a code block");
    insertNewNode2Parent(compound_rule, n->t, codeBlock); 
}

void parseClassDeclaration(treeNode* parent, tokenTable* table){
    treeNode* classDeclaration = insertNewNode2Parent(classDeclaration_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // optional access modifier
    peeknext = peekNextNode(table);
    if(isKey(PUBLIC, peeknext) || isKey(PRIVATE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(accessModifier_rule, n->t, classDeclaration);
    }
    
    // optional non-access modifier
    peeknext = peekNextNode(table);
    while(isKey(STATIC, peeknext) || isKey(FINAL, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(nonAccessModifier_rule, n->t, classDeclaration);
        peeknext = peekNextNode(table);
    }
    
    // optional abstract
    peeknext = peekNextNode(table);
    if(isKey(ABSTRACT, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(abstract_rule, n->t, classDeclaration);
    }
    
    // class
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, CLASS, "parseClassDeclaration", "missing class keyword");
    insertNewNode2Parent(class_rule, n->t, classDeclaration);
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseClassDeclaration", "missing class identifier");
    insertNewNode2Parent(identifier_rule, n->t, classDeclaration);
    
    // optional generics
    peeknext = peekNextNode(table);
    if(isSymbol('<', peeknext)){
        parseGenerics(classDeclaration, table);
    }
    
    // optionally extends one super class
    peeknext = peekNextNode(table);
    if(isKey(EXTENDS, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(extends_rule, n->t, classDeclaration);
        
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseClassDeclaration", "missing identifier for the super class to be extended");
        insertNewNode2Parent(identifier_rule, n->t, classDeclaration);
        
        peeknext = peekNextNode(table);
        if(isSymbol('<', peeknext)){
            parseGenerics(classDeclaration, table);
        }
    }
    
    // optionally implements some interfaces
    peeknext = peekNextNode(table);
    if(isKey(IMPLEMENTS, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(implements_rule, n->t, classDeclaration);
        
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseClassDeclaration", "missing identifier for the interface to be implemented");
        insertNewNode2Parent(identifier_rule, n->t, classDeclaration);
        
        peeknext = peekNextNode(table);
        if(isSymbol('<', peeknext)){
            parseGenerics(classDeclaration, table);
        }
        
        peeknext = peekNextNode(table);
        while(isSymbol(',', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(comma_rule, n->t, classDeclaration);
            
            n = nextNode(table);
            checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseClassDeclaration", "missing identifier for the interface to be implemented");
            insertNewNode2Parent(identifier_rule, n->t, classDeclaration);
            
            peeknext = peekNextNode(table);
            if(isSymbol('<', peeknext)){
                parseGenerics(classDeclaration, table);
            }
            peeknext = peekNextNode(table);
        }
    }
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseClassDeclaration", "missing left brace to start the body of the class");
    insertNewNode2Parent(compound_rule, n->t, classDeclaration);
    
    parseClassBody(classDeclaration, table);
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseClassDeclaration", "missing right brace to conclude the body of the class");
    insertNewNode2Parent(compound_rule, n->t, classDeclaration);
}

void parseInterfaceDeclaration(treeNode* parent, tokenTable* table){
    treeNode* interfaceDeclaration = insertNewNode2Parent(interfaceDeclaration_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // optional access modifier
    peeknext = peekNextNode(table);
    if(isKey(PUBLIC, peeknext) || isKey(PRIVATE, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(accessModifier_rule, n->t, interfaceDeclaration);
    }
    
    // interface
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, INTERFACE, "parseInterfaceDeclaration", "missing interface keyword");
    insertNewNode2Parent(interface_rule, n->t, interfaceDeclaration);
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseInterfaceDeclaration", "missing interface identifier");
    insertNewNode2Parent(identifier_rule, n->t, interfaceDeclaration);
    
    // optional generics
    peeknext = peekNextNode(table);
    if(isSymbol('<', peeknext)){
        parseGenerics(interfaceDeclaration, table);
    }
    
    // optionally extends some interfaces
    peeknext = peekNextNode(table);
    if(isKey(EXTENDS, peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(extends_rule, n->t, interfaceDeclaration);
        
        n = nextNode(table);
        checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseInterfaceDeclaration", "missing identifier for the interface to be extended");
        insertNewNode2Parent(identifier_rule, n->t, interfaceDeclaration);
        
        peeknext = peekNextNode(table);
        if(isSymbol('<', peeknext)){
            parseGenerics(interfaceDeclaration, table);
        }
        
        peeknext = peekNextNode(table);
        while(isSymbol(',', peeknext)){
            n = nextNode(table);
            insertNewNode2Parent(comma_rule, n->t, interfaceDeclaration);
            
            n = nextNode(table);
            checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseInterfaceDeclaration", "missing identifier for the interface to be extended");
            insertNewNode2Parent(identifier_rule, n->t, interfaceDeclaration);
            
            peeknext = peekNextNode(table);
            if(isSymbol('<', peeknext)){
                parseGenerics(interfaceDeclaration, table);
            }
            peeknext = peekNextNode(table);
        }
    }
    
    // '{'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '{', "parseInterfaceDeclaration", "missing left brace to start the body of the interface");
    insertNewNode2Parent(compound_rule, n->t, interfaceDeclaration);
    
    parseInterfaceBody(interfaceDeclaration, table);
    
    // '}'
    n = nextNode(table);
    checkCharValueNodeExpected(n, BRACKET, '}', "parseInterfaceDeclaration", "missing right brace to conclude the body of the interface");
    insertNewNode2Parent(compound_rule, n->t, interfaceDeclaration);
}    


void parseClassBody(treeNode* parent, tokenTable* table){
    treeNode* classbody = insertNewNode2Parent(classBody_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    while(!isBracket('}', peeknext)){
        if(isVariableDeclarationStart(peeknext)){
            printf("Debug: variable declaration at line %d\n", peeknext->t->lineNumber);
            parseVariableDeclaration(classbody, table);
            n = nextNode(table);
            checkCharValueNodeExpected(n, SEMICOLON, ';', "parseClassBody", "missing semicolon to conclude a variable declaration");
            insertNewNode2Parent(semicolon_rule, n->t, classbody);
        }
        else if(isSubroutineDeclarationStart(peeknext)){
            printf("Debug: subroutine declaration at line %d\n", peeknext->t->lineNumber);
            parseSubroutineDeclaration(classbody, table);
        }
        else{
            fprintf(stderr, "%sError parseClassBody line %d: unknown type of token in class body%s\n", RED, peeknext->t->lineNumber, NRM);
            exit(1);
        }
        peeknext = peekNextNode(table);
    }
}

void parseInterfaceBody(treeNode* parent, tokenTable* table){
    treeNode* interbody = insertNewNode2Parent(interfaceBody_rule, NULL, parent);
    tokenNode* peeknext;
    
    peeknext = peekNextNode(table);
    while(!isBracket('}', peeknext)){
        parseSubroutineDeclaration(interbody, table);
        peeknext = peekNextNode(table);
    }
}

void parseFile(treeNode* parent, tokenTable* table){
    // we do not create a new node for this parser
    tokenNode* peeknext;
    
    // import statements
    peeknext = peekNextNode(table);
    while(isKey(IMPORT, peeknext)){
        parseImportStatement(parent, table);
        peeknext = peekNextNode(table);
    }
    
    // class or interface
    peeknext = peekNextNode(table);
    while(peeknext){
        if(isClassStart(peeknext)){
            parseClassDeclaration(parent, table);
        }else if(isInterfaceStart(peeknext)){
            parseInterfaceDeclaration(parent, table);
        }else{
            fprintf(stderr, "%sError parseFile line %d: invalid token in file level%s\n", RED, peeknext->t->lineNumber, NRM);
            exit(1);
        }
        peeknext = peekNextNode(table);
    }
}

void parseImportStatement(treeNode* parent, tokenTable* table){
    treeNode* import = insertNewNode2Parent(importStatement_rule, NULL, parent);
    tokenNode* n;
    tokenNode* peeknext;
    
    // 'import'
    n = nextNode(table);
    checkKeyValueNodeExpected(n, KEYWORD, IMPORT, "parseImportStatement", "missing 'import' keyword");
    insertNewNode2Parent(import_rule, n->t, import);
    
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseImportStatement", "missing identifier after import");
    insertNewNode2Parent(identifier_rule, n->t, import);
    
    // if a '.' follows
    peeknext = peekNextNode(table);
    while(isSymbol('.', peeknext)){
        n = nextNode(table);
        insertNewNode2Parent(comma_rule, n->t, import);
        
        peeknext = peekNextNode(table);
        if(isIdentifier(peeknext)){
        
            n = nextNode(table);
            insertNewNode2Parent(identifier_rule, n->t, import);
            
        }else if(isSymbol('*', peeknext)){
        
            n = nextNode(table);
            insertNewNode2Parent(wildcard_rule, n->t, import);
            break;
            
        }else{
            fprintf(stderr, "%sError parseImportStatement line %d: unknown type of token%s\n", RED, peeknext->t->lineNumber, NRM);
            exit(1);
        }
        
        peeknext = peekNextNode(table);
    }
    
    // ';'
    n = nextNode(table);
    checkCharValueNodeExpected(n, SEMICOLON, ';', "parseImportStatement", "missing semicolon to conclude import");
    insertNewNode2Parent(semicolon_rule, n->t, import); 
}


