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
    
    printf("Debug: in expression only mode.\n")
    while(hasNext(table)){
        parseExpression(cst->root, table);
    }
}

void parseTerm(treeNode* parent, tokenTable* table){
    treeNode* term = insertNewNode2Parent("term", NULL, parent);
    
    parseBaseTerm(term, table);
    
    tokenNode* peeknext = peekNextNode(table);
    
    // use while loop to accomplish chaining accessing: obj.method1().field1.field2[2].method2()
    while(peeknext){
        if(peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='.'){
            parseFieldAccess(term, table);
        }
        else if(peeknext->t->type==BRACKET && peeknext->t->data.char_val=='['){
            parseArrayAccess(term, table);
        }
        else if(peeknext->t->type==BRACKET && peeknext->t->data.char_val=='('){
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
    }
}

void parseFieldAccess(treeNode* parent, tokenTable* table){
    treeNode* field = insertNewNode2Parent("fieldAccess", NULL, parent);
    
    // dot
    toKenNode* n = nextNode(table);
    insertNewNode2Parent("symbol", n->t, field);
    
    // identifier
    n = nextNode(table);
    checkStringValueNodeExpected(n, IDENTIFIER, NULL, "parseFieldAccess", "identifier expected after dot");
    insertNewNode2Parent("identifier", n->t, field);
}

void parseArrayAccess(treeNode* parent, tokenTable* table){
    treeNode* array = insertNewNode2Parent("arrayAccess", NULL, parent);
    
    // left bracket
    toKenNode* n = nextNode(table);
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
    toKenNode* n = nextNode(table);
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
    tokenNode* n = nextNode(table);
    checkCharValueNodeExpected(n, SYMBOL, ':', "parseTernaryExpression", "missing colon in ternary expression");
    insertNewNode2Parent("symbol", n->t, ternary);
    
    parseExpression(ternary, table);
}

void parseLogicalOrExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalOr = insertNewNode2Parent("logicalOrExpression", NULL, parent);
    
    parseLogicalAndExpression(logicalOr, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==OPERATOR && !strcmp(peeknext->t->str_val, "||")){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, logicalOr);
        
        parseLogicalAndExpression(logicalOr, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseLogicalAndExpression(treeNode* parent, tokenTable* table){
    treeNode* logicalAnd = insertNewNode2Parent("logicalAndExpression", NULL, parent);
    
    parseEqualityExpression(logicalAnd, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==OPERATOR && !strcmp(peeknext->t->str_val, "&&")){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, logicalAnd);
        
        parseEqualityExpression(logicalAnd, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseEqualityExpression(treeNode* parent, tokenTable* table){
    treeNode* equality = insertNewNode2Parent("equalityExpression", NULL, parent);
    
    parseRelationalExpression(equality, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->str_val, "==") || !strcmp(peeknext->t->str_val, "!=")) ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, equality);
        
        parseRelationalExpression(equality, table);
        
        peeknext = peekNextNode(table);
    }
}

void parseRelationalExpression(treeNode* parent, tokenTable* table){
    treeNode* relational = insertNewNode2Parent("relationalExpression", NULL, parent);
    
    parseAdditiveExpression(relational, table);
    
    tokenNode* peeknext = peekNextNode(table);
    if(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->str_val, "<=") || !strcmp(peeknext->t->str_val, ">=")) ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, relational);
        
        parseAdditiveExpression(relational, table);

        return;
    }
    else if(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->char_val=='>' || peeknext->t->char_val=='<') ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, relational);
        
        parseAdditiveExpression(relational, table);

        return;
    }
    else if(peeknext && peeknext->t->type==KEYWORD && peeknext->t->key_val==INSTANCEOF){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("keyword", n->t, relational);
        
        parseReferenceType(relational, table);

        return;
    }
}

void parseAdditiveExpression(treeNode* parent, tokenTable* table){
    treeNode* additive = insertNewNode2Parent("additiveExpression", NULL, parent);
    
    parseMultiplicativeExpression(additive, table);
    
    tokenNode* peeknext = peekNextNode(table);
    while(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->char_val=='+' || peeknext->t->char_val=='-') ){
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
    while(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->char_val=='*' || peeknext->t->char_val=='/' || peeknext->t->char_val=='%') ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, multiplicative);
        
        parseUnaryExpression(multiplicative, table);
        
        peeknext = peekNextNode(table);
    }
}
void parseUnaryExpression(treeNode* parent, tokenTable* table){
    treeNode* unary = insertNewNode2Parent("unaryExpression", NULL, parent);
    
    tokenNode* peeknext = peekNextNode(table);
    if(peeknext && peeknext->t->type==SYMBOL && (peeknext->t->char_val=='!' || peeknext->t->char_val=='-' || peeknext->t->char_val=='~') ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("symbol", n->t, unary);
        
        parseUnaryExpression(unary, table);

        return;
    }
    else if(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->str_val, "++") || !strcmp(peeknext->t->str_val, "--")) ){
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
    
    if(peeknext && peeknext->t->type==OPERATOR && (!strcmp(peeknext->t->str_val, "++") || !strcmp(peeknext->t->str_val, "--")) ){
        tokenNode* n = nextNode(table);
        insertNewNode2Parent("operator", n->t, postfix);
    }
}

void parseReferenceType(treeNode* parent, tokenTable* table){
    
}
