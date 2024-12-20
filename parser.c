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
    if(!n){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create rule type string.\n", t->lineNumber);
        exit(1);
    } 
}

treeNode** insertTreeNode(treeNode** children, int capacity, int childCount){
    
}
