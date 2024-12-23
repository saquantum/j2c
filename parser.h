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
void parseTernaryExpression(treeNode* parent, tokenTable* table);
void parseLogicalOrExpression(treeNode* parent, tokenTable* table);
void parseLogicalAndExpression(treeNode* parent, tokenTable* table);
void parseBitwiseOrExpression(treeNode* parent, tokenTable* table);
void parseBitwiseXorExpression(treeNode* parent, tokenTable* table);
void parseBitwiseAndExpression(treeNode* parent, tokenTable* table);
void parseEqualityExpression(treeNode* parent, tokenTable* table);
void parseRelationalExpression(treeNode* parent, tokenTable* table);
void parseShiftExpression(treeNode* parent, tokenTable* table);
void parseAdditiveExpression(treeNode* parent, tokenTable* table);
void parseMultiplicativeExpression(treeNode* parent, tokenTable* table);
void parseUnaryExpression(treeNode* parent, tokenTable* table);
void parsePostfixExpression(treeNode* parent, tokenTable* table);

void parseType(treeNode* parent, tokenTable* table);
void parseReferenceType(treeNode* parent, tokenTable* table);
void parseGenerics(treeNode* parent, tokenTable* table);
void parseTypeArgument(treeNode* parent, tokenTable* table);

void parseAssignment(treeNode* parent, tokenTable* table);
void parseVariableDeclaration(treeNode* parent, tokenTable* table);
void parseSubroutineDeclaration(treeNode* parent, tokenTable* table);
void parseParameterList(treeNode* parent, tokenTable* table);
void parseSubroutineBody(treeNode* parent, tokenTable* table);

void parseStatement(treeNode* parent, tokenTable* table);
void parseIfStatement(treeNode* parent, tokenTable* table);
void parseSwitchStatement(treeNode* parent, tokenTable* table);
void parseForStatement(treeNode* parent, tokenTable* table);
void parseWhileStatement(treeNode* parent, tokenTable* table);
void parseDoWhileStatement(treeNode* parent, tokenTable* table);
void parseReturnStatement(treeNode* parent, tokenTable* table);

/* parser helpers */

bool isExpressionStart(token* t);

bool isKey(keyword Key, tokenNode* n);
bool isSymbol(char c, tokenNode* n);
bool isIdentifier(tokenNode* n);
bool isOperator(char* o, tokenNode* n);
bool isBracket(char c, tokenNode* n);
bool isNumber(tokenNode* n);
bool isString(tokenNode* n);
bool isSemicolon(tokenNode* n);

bool isPotentialGenerics(tokenNode* current);
bool isPotentialAssignment(tokenNode* current);
bool isPotentialVariableDeclaration(tokenNode* current);
bool isPotentialStatement(tokenNode* current);

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

void printCST(CST* cst);
void printTreeNode(treeNode* n);

void freeCST(CST** cst);
void freeTreeNode(treeNode* n);
