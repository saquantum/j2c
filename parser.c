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
        insertNewNode2Parent("termTerminal", n->t, parent);
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
    
    
}

void parseExpressionList(treeNode* parent, tokenTable* table){

}

void parseExpression(treeNode* parent, tokenTable* table){

}

