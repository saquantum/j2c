//#pragma once
#include "symboltable.h"

bool checkCSTM(classSTManager* cstm);
bool checkDuplicateClass(classSTManager* cstm);

bool checkClass(classSTManager* cstm, classST* st);
bool checkClassInheritance(classSTManager* cstm, classST* st);
bool checkInterfaceImplementation(classSTManager* cstm, classST* st);
bool checkCyclicImplementation(classSTManager* cstm, classST* st, classST* current);
bool checkAbstract(classSTManager* cstm,classST* st);

bool checkMethods(classSTManager* cstm, classST* st);
bool checkDuplicateArgument(methodST* st);
bool isEmptyMethod(methodST* st);
bool checkMethodTree(classSTManager* cstm, methodST* st);
bool checkMethodTreeHelper(classSTManager* cstm, treeNode* n);
bool checkVariableDeclaration(classSTManager* cstm, treeNode* n);
int countArrayDimension(classSTManager* cstm, treeNode* n);
bool checkAssignmentCompatible(classSTManager* cstm, treeNode* n);

int typeOfTerm(classSTManager* cstm, treeNode* n);
int typeOfIdentifier(classSTManager* cstm, treeNode* n, treeNode* parentMethod, treeNode* parentClass);
int typeOfExpression(classSTManager* cstm, treeNode* n);
bool isvalidInheritance(classSTManager* cstm, int super, int sub);

bool checkStatement(classSTManager* cstm, treeNode* n);

