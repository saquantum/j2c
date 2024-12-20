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

bool isBinaryOp(token* t);

bool isLogicalOp(token* t);

bool isLogicalBindOp(token* t);

bool isRelationalOp(token* t);

bool isShiftOp(token* t);

bool isAssignmentOp(token* t);

bool isSelfOp(token* t);

treeNode* insertNewNode2Parent(char* rule, token* t, treeNode* parent);

// this function never returns null pointer, it only crashes.
// so we don't need null pointer check after calling this function.
treeNode* createTreeNode(char* rule, token* t);

void insertChildNode(treeNode* n, treeNode* child);

void freeCST(CST** cst);

void freeTreeNode(treeNode* n);
