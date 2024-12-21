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

/* parsers */

CST* parseTokenTable(char* filename, tokenTable* table);

void parseTerm(treeNode* parent, tokenTable* table);
void parseBaseTerm(treeNode* parent, tokenTable* table);
void parseFieldAccess(treeNode* parent, tokenTable* table);
void parseArrayAccess(treeNode* parent, tokenTable* table);
void parseSubroutineCall(treeNode* parent, tokenTable* table);
void parseExpressionList(treeNode* parent, tokenTable* table);

void parseExpression(treeNode* parent, tokenTable* table);

/* operators */

bool isBinaryOp(token* t);

bool isLogicalOp(token* t);

bool isLogicalBindOp(token* t);

bool isRelationalOp(token* t);

bool isShiftOp(token* t);

bool isAssignmentOp(token* t);

bool isSelfOp(token* t);

/* tree and nodes */

void checkKeyValueNodeExpected(tokenNode* n, tokenType expectedType, keyword expectedValue, char* functionName, char* errorMessage);

void checkCharValueNodeExpected(tokenNode* n, tokenType expectedType, char expectedValue, char* functionName, char* errorMessage);

void checkStringValueNodeExpected(tokenNode* n, tokenType expectedType, char* expectedValue, char* functionName, char* errorMessage);

// create a child node with rule and token, and insert it to parent.
treeNode* insertNewNode2Parent(char* rule, token* t, treeNode* parent);

// this function never returns null pointer, it only crashes.
// so we don't need null pointer check after calling this function.
treeNode* createTreeNode(char* rule, token* t);

void insertChildNode(treeNode* n, treeNode* child);

void freeCST(CST** cst);

void freeTreeNode(treeNode* n);
