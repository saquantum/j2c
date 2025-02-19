#pragma once
#include "lexer.h"

typedef enum ruletype{
    identifier_rule,
    bracket_rule,
    compound_rule,
    operator_rule,
    semicolon_rule,
    
    type_rule,primitiveType_rule,referenceType_rule,
    generics_rule,typeArgument_rule,wildcard_rule,
    langle_rule,rangle_rule,
    comma_rule,
    extends_rule,super_rule,implements_rule,
    annotation_rule,at_rule,
    term_rule,
    parenthesizedExpression_rule,
    newObject_rule,new_rule,
    arrayInitialization_rule,
    terminalTerm_rule,number_rule,character_rule,string_rule,
    dot_rule,fieldAccess_rule,arrayAccess_rule,
    subroutineCall_rule,expressionList_rule,
    expression_rule,
    ternaryExpression_rule,ternaryOperator_rule,
    logicalOrExpression_rule,
    logicalAndExpression_rule,
    bitwiseOrExpression_rule,
    bitwiseXorExpression_rule,
    bitwiseAndExpression_rule,
    equalityExpression_rule,
    relationalExpression_rule,instanceof_rule,
    shiftExpression_rule,
    additiveExpression_rule,
    multiplicativeExpression_rule,
    castExpression_rule,
    unaryExpression_rule,
    postfixExpression_rule,
    assignment_rule,assignmentOperator_rule,
    variableDeclaration_rule,
    accessModifier_rule,nonAccessModifier_rule,
    subroutineDeclaration_rule,
    native_rule,
    parameterList_rule,const_rule,
    typeBoundList_rule,typeBound_rule,constraint_rule,moreInterface_rule,
    subroutineBody_rule,
    statement_rule,
    ifStatement_rule,if_rule,else_rule,
    switchStatement_rule,switch_rule,case_rule,colon_rule,default_rule,
    forStatement_rule,for_rule,
    whileStatement_rule,while_rule,
    doWhileStatement_rule,do_rule,
    returnStatement_rule,return_rule,
    continueStatement_rule,continue_rule,
    breakStatement_rule,break_rule,
    staticStatement_rule,static_rule,
    codeBlock_rule,
    classDeclaration_rule,abstract_rule,class_rule,
    interfaceDeclaration_rule,interface_rule,
    classBody_rule,interfaceBody_rule,
    subroutinePrototype_rule,
    importStatement_rule,import_rule,
    file_rule
}ruletype;

/* forward declaration */

struct classST;
struct methodST;
struct varST;

typedef struct treeNode{
    // node info --------
    ruletype ruleType;
    token* assoToken;
    struct treeNode** children;
    size_t capacity;
    size_t childCount;
    struct treeNode* parent;
    // symbol table --------
    struct classST* classSymbolTable; // attach this for a structural class node
    struct methodST* methodSymbolTable; // attach this for a structural method node
    struct varST** varSymbolTable; // attach this for a structural compound node
    size_t varCount;
}treeNode;

// every .java file has one CST.
typedef struct CST{
    treeNode* root;
    char* filename;
}CST;

/* parsers */

CST* parseTokenTable(char* filename, tokenTable* table);

void parseAnnotation(treeNode* parent, tokenTable* table);
void parseTerm(treeNode* parent, tokenTable* table);
void parseNewObject(treeNode* parent, tokenTable* table);
void parseArrayInitialization(treeNode* parent, tokenTable* table);
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
void parseCastExpression(treeNode* parent, tokenTable* table);
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
void parseTypeBoundList(treeNode* parent, tokenTable* table);
void parseTypeBound(treeNode* parent, tokenTable* table);
void parseConstraint(treeNode* parent, tokenTable* table);
void parseSubroutineBody(treeNode* parent, tokenTable* table);

void parseStatement(treeNode* parent, tokenTable* table);
void parseIfStatement(treeNode* parent, tokenTable* table);
void parseSwitchStatement(treeNode* parent, tokenTable* table);
void parseForStatement(treeNode* parent, tokenTable* table);
void parseWhileStatement(treeNode* parent, tokenTable* table);
void parseDoWhileStatement(treeNode* parent, tokenTable* table);
void parseReturnStatement(treeNode* parent, tokenTable* table);
void parseContinueStatement(treeNode* parent, tokenTable* table);
void parseBreakStatement(treeNode* parent, tokenTable* table);
void parseStaticStatement(treeNode* parent, tokenTable* table);
void parseCodeBlock(treeNode* parent, tokenTable* table);

void parseClassDeclaration(treeNode* parent, tokenTable* table);
void parseInterfaceDeclaration(treeNode* parent, tokenTable* table);
void parseClassBody(treeNode* parent, tokenTable* table);
void parseInterfaceBody(treeNode* parent, tokenTable* table);
void parseSubroutinePrototype(treeNode* parent, tokenTable* table);
void parseFile(treeNode* parent, tokenTable* table);
void parseImportStatement(treeNode* parent, tokenTable* table);

/* parser helpers */

bool isKey(keyword Key, tokenNode* n);
bool isSymbol(char c, tokenNode* n);
bool isIdentifier(tokenNode* n);
bool isOperator(char* o, tokenNode* n);
bool isBracket(char c, tokenNode* n);
bool isNumber(tokenNode* n);
bool isCharacter(tokenNode* n);
bool isString(tokenNode* n);
bool isSemicolon(tokenNode* n);

bool isExpressionStart(tokenNode* current);
bool isPotentialGenerics(tokenNode* current);
bool isPotentialAssignment(tokenNode* current);
bool isVariableDeclarationStart(tokenNode* current);
bool isPotentialSubroutineCall(tokenNode* current);
bool isStatementStart(tokenNode* current);
bool isClassStart(tokenNode* current);
bool isInterfaceStart(tokenNode* current);
bool isPotentialType(tokenNode* current);
bool isSubroutineDeclarationStart(tokenNode* current);
bool isPotentialCasting(tokenNode* current);

/* tree and nodes */

void checkKeyValueNodeExpected(tokenNode* n, tokenType expectedType, keyword expectedValue, char* functionName, char* errorMessage);
void checkCharValueNodeExpected(tokenNode* n, tokenType expectedType, char expectedValue, char* functionName, char* errorMessage);
void checkStringValueNodeExpected(tokenNode* n, tokenType expectedType, char* expectedValue, char* functionName, char* errorMessage);

// create a child node with rule and token, and insert it to parent.
treeNode* insertNewNode2Parent(ruletype rule, token* t, treeNode* parent);

// this function never returns null pointer, it only crashes.
// so we don't need null pointer check after calling this function.
treeNode* createTreeNode(ruletype rule, token* t);

void insertChildNode(treeNode* n, treeNode* child);

void printCST(CST* cst);
void printTreeNode(treeNode* n, int indent);
void printLessCST(CST* cst);
void printLessTreeNode(treeNode* n, int indent);

char* getRule(ruletype r);

void freeCST(CST** cst);
void freeTreeNode(treeNode* n);

treeNode* deepCopySubtree(treeNode* n);
