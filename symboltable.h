#pragma once
#include "parser.h"

void attachSymbolTables(CST* cst);
void attachSymbolTables2Nodes(treeNode* n);
// the formal node of class need a classST with nested methodST and varST.
// the formal node of a method need a methodST with nested varST.
// the formal node of a compound need a varST.
// hierarchy: classST <- methodST <- varST (compound) <- varST (nested compound) <- ...
//                 ^        ^<-- varST (method local var)
//                 |-- varST (class level var)

classST* attachClassSymbolTable(treeNode* n);
methodST* attachMethodSymbolTable(treeNode* n, treeNode* parentClass);
genST* attachTypeBoundSymbolTable(char* name, treeNode* typeBoundList);
treeNode* convertTypeBound2Generics(treeNode* typeBoundList);
varST** attachVarSymbolTable(treeNode* n, treeNode* parentClass, 
        treeNode* parentMethod, treeNode* parentCompound);
varST** attachArgListSymbolTable(treeNode* n, treeNode* parentMethod);
genST* attachGenericsSymbolTable(char* type, treeNode* gen);



int countCommas(treeNode* n);
int countBrackets(char c1, char c2, treeNode* n);

void printSymbolTables(CST* cst);
void printNodeSymbolTable(treeNode* n, int indent);
void printClassST(classST* st);
void printMethodST(methodST* st);
void printVarST(varST* st);
void printGenericsST(genST* st);

void freeSymbolTables(CST* cst);
void freeNodeSymbolTables(treeNode* n);
void freeClassST(classST* st);
void freeMethodST(methodST* st);
void freeVarST(varST* st);
void freeGenericsST(genST* st);

void* testcalloc(size_t len, size_t size);

