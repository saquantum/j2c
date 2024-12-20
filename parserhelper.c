#include "parser.h"

bool isBinaryOp(token* t){
    if(!t){
        return false;
    }
    if(t->type==SYMBOL){
        return t->data.char_val=='+' || t->data.char_val=='-' || t->data.char_val=='*' || t->data.char_val=='/' || t->data.char_val=='%' || t->data.char_val=='^' || t->data.char_val=='&' || t->data.char_val=='|';
    }
    if(t->type==OPERATOR){
        return isShiftOp(t);
    }
    return false;
}

bool isLogicalOp(token* t){
    if(!t){
        return false;
    }
    if(t->type==SYMBOL){
        return t->data.char_val=='<' || t->data.char_val=='>';
    }
    if(t->type==OPERATOR){
        return isLogicalBindOp(t) || isRelationalOp(t);
    }
    return false;
}

bool isLogicalBindOp(token* t){
    if(!t || t->type!=OPERATOR || strlen(t->data.str_val)!=3){
        return false;
    }
    return !strcmp(t->data.str_val, "&&") || !strcmp(t->data.str_val, "||");
}

bool isRelationalOp(token* t){
    if(!t || t->type!=OPERATOR || strlen(t->data.str_val)!=3){
        return false;
    }
    return !strcmp(t->data.str_val, "==") || !strcmp(t->data.str_val, "!=") || !strcmp(t->data.str_val, ">=") || !strcmp(t->data.str_val, "<=");
}

bool isShiftOp(token* t){
    if(!t || t->type!=OPERATOR || strlen(t->data.str_val)!=3){
        return false;
    }
    return !strcmp(t->data.str_val, ">>") || !strcmp(t->data.str_val, "<<");
}

bool isAssignmentOp(token* t){
    if(!t || t->type!=OPERATOR || strlen(t->data.str_val)!=3){
        return false;
    }
    return !strcmp(t->data.str_val, "+=") || !strcmp(t->data.str_val, "-=") || !strcmp(t->data.str_val, "*=") || !strcmp(t->data.str_val, "/=");
}

bool isSelfOp(token* t){
    if(!t || t->type!=OPERATOR || strlen(t->data.str_val)!=3){
        return false;
    }
    return !strcmp(t->data.str_val, "++") || !strcmp(t->data.str_val, "--");
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
    freeTreeNode((*cst)->root);
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
