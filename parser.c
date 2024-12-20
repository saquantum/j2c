#include "parser.h"

// public static final int x = 1 + 2 , y = 10 , z ;
// ArrayList<String> list = new ArrayList<>();
void parseLetStatement(tokenNode* n0, tokenNode* nf){
    
}

typedef struct treeNode{
    char* ruleType;
    token* assoToken;
    struct treeNode** children;
    int capacity;
    int childCount;
    struct treeNode* parent;
}treeNode;

treeNode* createTreeNode(char* rule, token* t){
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
}

void insertChildNode(treeNode* n, treeNode* child){
    if(n->capacity < n->childCount){
        if(n->assoToken){
            fprintf(stderr, "Error insertChildNode line %d: code of parser has destructively wrong logic.\n", n->assoToken->lineNumber);
        }else{
            fprintf(stderr, "Error insertChildNode: code of parser has destructively wrong logic.\n");
        }
        exit(1);
    }
    if(n->capacity == n->childCount){
        n->children = (treeNode**)realloc(n->children, 2*(n->capacity));
        if(!n->children){
            if(n->assoToken){
                fprintf(stderr, "Error insertChildNode line %d: not enough memory, cannot realloc children array.\n", n->assoToken->lineNumber);
            }else{
                fprintf(stderr, "Error insertChildNode: not enough memory, cannot realloc children array.\n");
            }
            exit(1);
        }
    }
    n->children[n->childcount++] = child;
}
