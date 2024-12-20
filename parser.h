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

typedef struct CST{
    
}CST;

treeNode** insertTreeNode(treeNode** children, int capacity, int childCount);

treeNode* createTreeNode(char* rule, token* t);
