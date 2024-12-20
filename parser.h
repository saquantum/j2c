#pragma once
#include "lexer.h"

typedef struct treeNode{
    char* ruleType;
    token* assoToken;
    struct treeNode** children;
    int capacity;
    int childCount;
    struct treeNode* parent;
}treeNode;

// every .java file has one CST.
typedef struct CST{
    treeNode* root;
}CST;



treeNode* createTreeNode(char* rule, token* t);

void insertChildNode(treeNode* n, treeNode* child);
