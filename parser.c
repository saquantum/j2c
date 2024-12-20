#include "parser.h"

//void parseExpression(treeNode* parent, tokenTable* table){}

void parseTerm(treeNode* parent, tokenTable* table){
    tokenNode* n = nextNode(table);
    // four terminal cases: true, false, null, this
    if(n->t->type==KEYWORD && (n->t->data.key_val==BOOL_TRUE || n->t->data.key_val==BOOL_FALSE || n->t->data.key_val==NULLER || n->t->data.key_val==THIS)){
        insertNewNode2Parent("term", n->t, parent);
        return;
    }
    // token is a number or string
    else if(n->t->type==NUMBER){
        treeNode* term = insertNewNode2Parent("term", NULL, parent);
        insertNewNode2Parent("number", n->t, term);
        return;
    }
    else if(n->t->type==STRING){
        treeNode* term = insertNewNode2Parent("term", NULL, parent);
        insertNewNode2Parent("string", n->t, term);
        return;
    }
    // term is a variable or an array entry, or a variable with post self in/decrement, or a subroutine call
    else if(n->t->type==IDENTIFIER){
        tokenNode* peeknext = peekNextNode(table);
        if(peeknext->t->type==BRACKET && peeknext->t->data.char_val=='['){
        
            treeNode* term = insertNewNode2Parent("term", NULL, parent);
            
            insertNewNode2Parent("identifier", n->t, term);
            
            n = nextNode(table);
            insertNewNode2Parent("bracket", n->t, term);
            
            parseExpression(term, table);
            
            n = nextNode(table);
            if(n->t->type!=BRACKET || n->t->data.char_val!=']'){
                fprintf(stderr, "Error parseTerm line %d: missing right square bracket.\n", n->t->lineNumber);
                exit(1);
            }
            insertNewNode2Parent("bracket", n->t, term);
            return;
        } else if(isSelfOp(peeknext->t)){
        
            treeNode* term = insertNewNode2Parent("term", NULL, parent);
            
            insertNewNode2Parent("identifier", n->t, term);
            
            n = nextNode(table);
            insertNewNode2Parent("selfOperator", n->t, term);
            
            return;
        } else if ((peeknext->t->type==BRACKET && peeknext->t->data.char_val=='(') || (peeknext->t->type==SYMBOL && peeknext->t->data.char_val=='.')){
            unread(table);
            treeNode* term = insertNewNode2Parent("term", NULL, parent);
            parseSubroutineCall(term, table);
        }else{
            insertNewNode2Parent("term", n->t, parent);
            return;
        }
    }
    // term is an expression within a round bracket
    else if(n->t->type==BRACKET && n->t->data.char_val=='('){
        treeNode* term = insertNewNode2Parent("term", NULL, parent);
        
        insertNewNode2Parent("bracket", n->t, term);
        
        parseExpression(term, table);
        
        n = nextNode(table);   
        if(n->t->type!=BRACKET || n->t->data.char_val!=')'){
            fprintf(stderr, "Error parseTerm line %d: missing right round bracket.\n", n->t->lineNumber);
            exit(1);
        }
        insertNewNode2Parent("bracket", n->t, term);
        return;
    }
    // term is an unary operator acting on another term
    else if(n->t->type==SYMBOL && (n->t->data.char_val=='!' || n->t->data.char_val=='~' || n->t->data.char_val=='-')){
        treeNode* term = insertNewNode2Parent("term", NULL, parent);
        
        insertNewNode2Parent("unaryOperator", n->t, term);
        
        parseTerm(term, table);

        return;
    }
    // term is a pre in/decremented identifier
    else if(isSelfOp(n->t)){
        treeNode* term = insertNewNode2Parent("term", NULL, parent);
            
        insertNewNode2Parent("selfOperator", n->t, term);
        
        n = nextNode(table);
            if(n->t->type!=IDENTIFIER){
                fprintf(stderr, "Error parseTerm line %d: missing identifier.\n", n->t->lineNumber);
                exit(1);
            }
        insertNewNode2Parent("identifier", n->t, term);
    }
    // term is a ternary expression
    else{
        unread(table);
        treeNode* term = insertNewNode2Parent("term", NULL, parent);
        
        parseExpression(term, table);
        
        n = nextNode(table);
            if(n->t->type!=SYMBOL || n->t->data.char_val!='?'){
                fprintf(stderr, "Error parseTerm line %d: missing '?' in ternary expression.\n", n->t->lineNumber);
                exit(1);
            }
        insertNewNode2Parent("symbol", n->t, term);
        
        parseExpression(term, table);
        
        n = nextNode(table);
            if(n->t->type!=SYMBOL || n->t->data.char_val!=':'){
                fprintf(stderr, "Error parseTerm line %d: missing ':' in ternary expression.\n", n->t->lineNumber);
                exit(1);
            }
        insertNewNode2Parent("symbol", n->t, term);
        
        parseExpression(term, table);
    }
    
}

