#include "parser.h"

void parseExpression(treeNode* parent, tokenTable* table){
    
}

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
    // term is a variable or an array entry
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
        } else{
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
    else if(n->t->type==SYMBOL && (n->t->data.char_val=='!' | n->t->data.char_val=='~' | (n->t->data.char_val=='-' && peekNextNode(table)->t->type!=SYMBOL ))){}

}

treeNode* insertNewNode2Parent(char* rule, token* t, treeNode* parent){
    treeNode* child = createTreeNode(rule, t);
    child->parent = parent;
    insertChildNode(parent, child);
    return child;
}

treeNode* createTreeNode(char* rule, token* t){
    if(!rule){
        fprintf(stderr, "Error createTreeNode: null rule provided.\n");
        exit(1);
    }
    treeNode* n = calloc(1,sizeof(treeNode));
    if(!n){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create tree node.\n", t->lineNumber);
        exit(1);
    } 
    n->ruleType = calloc((int)strlen(rule)+1, sizeof(char));
    if(!n->ruleType){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create rule type string.\n", t->lineNumber);
        exit(1);
    } 
    strcpy(n->ruleType, rule);
    n->assoToken = t;
    n->children = calloc(16, sizeof(treeNode*));
    if(!n->children){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create children array.\n", t->lineNumber);
        exit(1);
    } 
    n->capacity = 16;
    return n;
}

void insertChildNode(treeNode* n, treeNode* child){
    if(!n || !child){
        fprintf(stderr, "Error insertChildNode: null pointer provided.\n");
        exit(1);
    }
    if(n->capacity < n->childCount){
        if(n->assoToken){
            fprintf(stderr, "Error insertChildNode line %d: code of parser has destructively wrong logic.\n", n->assoToken->lineNumber);
        }else{
            fprintf(stderr, "Error insertChildNode: code of parser has destructively wrong logic.\n");
        }
        exit(1);
    }
    if(n->capacity == n->childCount){
        n->children = (treeNode**)realloc(n->children, 2*(n->capacity)*sizeof(treeNode*));
        if(!n->children){
            if(n->assoToken){
                fprintf(stderr, "Error insertChildNode line %d: not enough memory, cannot realloc children array.\n", n->assoToken->lineNumber);
            }else{
                fprintf(stderr, "Error insertChildNode: not enough memory, cannot realloc children array.\n");
            }
            exit(1);
        }
        n->capacity = 2 * (n->capacity);
    }
    n->children[n->childCount++] = child;
    child->parent = n;
}

void freeCST(CST** cst){
    if(!cst || !(*cst)){
        return;
    }
    freeTreeNode(cst->root);
    *cst = NULL;
}

void freeTreeNode(treeNode* n){
    if(!n){
        return;
    }
    for(int i=0; i < n->childCount; i++){
        freeTreeNode(n->children[i]);
    }
    free(n->children);
    free(n->ruleType);
    free(n);
}
