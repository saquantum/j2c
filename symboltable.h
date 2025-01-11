#pragma once
#include "parser.h"

/* forward declaration */
struct treeNode;

typedef struct vtable{
    struct methodST** entries;
    size_t entryCount;
    struct vtable* super;
    struct classST* attachClass;
} vtable;

typedef enum classification_of_ST{
    CLASS_ST,
    METHOD_ST,
    VAR_ST,
    GEN_ST
} classification_of_ST;

// to access the name of a class, retrieve classST->generics->type
typedef struct classST{
    classification_of_ST cf; // should be set to CLASS_ST by default
    
    bool isClass;
    bool isInterface; // one and only one of them can be true, the other must be false
    
    struct genST* generics; // class level generics
    
    struct genST* superclassGenerics; // if not designated during parsing, it's Object
    
    struct genST** interfacesGenerics; // if not designated during parsing, it's NULL
    size_t interfacesCount;
    
    struct varST** fields;
    size_t fieldsCount;
    
    struct methodST** methods;
    size_t methodsCount;
    
    bool isPublic;
    bool isPrivate;
    bool isAbstract;
    bool isStatic;
    bool isFinal;
    
    bool isRunnable; // if it has main has an interior method
    
    struct treeNode* attachNode;
    vtable* virtualTable; // for an interface, keep a 'vtable' to retrieve all methods quickly
} classST;

// to access the name of a method, retrieve methodST->generics->type
typedef struct methodST{
    classification_of_ST cf; // should be set to METHOD_ST by default
    
    char* annotation;
    char* name;
    // if subroutine is a constructor, set returnType to NULL and isConstructor to true
    // for ordinary method, isConstructor to false
    struct genST* returnType;
    bool returnsNumber;
    bool returnsBool;
    bool isConstructor;
    
    struct genST* generics; // type boundedness for this method, this affects arguments
    size_t arrDimension; // if the return value is an array, then it's not zero
    
    struct varST** arguments; // array of arguments of this method, NULL -> no argument
    size_t argumentsCount;
    
    struct varST** locals; // array of local variables, NULL -> no local
    size_t localsCount;
    
    bool isPublic;
    bool isPrivate;
    int access; // public->4, default->2, private->1
    
    bool isAbstract;
    bool isStatic;
    bool isFinal;
    
    // refer to corresponding C source code if it is native
    bool isNative;
    
    struct treeNode* parentClass;
    struct treeNode* attachNode;
    
}methodST;

// Map<? extends Comparable<?> , V extends List<String> >
typedef struct genST{
    classification_of_ST cf; // should be set to GEN_ST by default
    char* name; // the identifier before extends or super. Map< ... , ... > -> name = NULL
    bool isWildcard;
    bool extends;
    bool super;
    char* type; // Map is the type!
    struct genST** nested; // to next level. {name="?", extends=true, super=false, type=Comparable, nested={name=NULL, extends=false, super=false, type="?"}}, {name="V", extends=true, super=false, type=List, nested={name=NULL, extends=false, super=false, type="String"}}.
    size_t nestedCount;
}genST;

typedef struct varST{
    classification_of_ST cf; // should be set to VAR_ST by default
    char* name;
    struct genST* type; // type of a variable is with generics by default
    
    // array dimension determines number of asterisks for a pointer
    size_t arrDimension; // if 0, not an array
    
    bool isPublic;
    bool isPrivate;
    bool isStatic;
    bool isFinal;
    
    bool isPrimitive;
    
    struct treeNode* parentClass;
    struct treeNode* parentMethod;
    struct treeNode* parentCompound;
    struct treeNode* attachNode;
}varST;

typedef struct classSTManager{
    // if we need a better lookup should use linked hash list but im lazy
    classST** registeredTables;
    size_t length;
    size_t capacity;
    bool hasMain;
}classSTManager;

/* Symbol tables */

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
int countType(treeNode* n);
int countBrackets(char c1, char c2, treeNode* n);

/* virtual tables */

classSTManager* createCSTM();
void insert2CSTM(classSTManager* cstm, classST* st);
void insertClass2CSTM(classSTManager* cstm, CST* cst);
void insertClass2CSTMHelper(classSTManager* cstm, treeNode* n);
classST* lookupClassST(classSTManager* cstm, char* className);
int lookupIndexClassST(classSTManager* cstm, char* className);
int lookupVarInClass(classST* st, char* varName);
int lookupVarInMethod(methodST* st, char* varName);
bool isVirtualMethod(methodST* st);
void assignUniqueName(classST* st);
// turn accessibility into a number
int methodAccess(methodST* st);
// decide if two genST are the same
bool areGenericsEqual(genST* st1, genST* st2, int mode);
// decide if method1 can override method2
bool methodOverridesSimpleChecks(methodST* st1, methodST* st2);
bool methodOverrides(methodST* st1, methodST* st2);
bool compareUnorderedGenSTArrays(size_t size, genST** arr1, genST** arr2, int mode);
// according to the information on the class that st extends from cstm, create st's vtable
vtable* attachVirtualTable(classSTManager* cstm, classST* st);
// checks if method1 has a smaller return type
bool isValidOverrideReturnType(methodST* st1, methodST* st2, classSTManager* cstm);
// helper
bool isCompatibleInterface(classST* st1, classST* st2, classSTManager* cstm);
// checks if method does not override but has override annotation
bool hasOverride(methodST* st);
bool implementsInterface(classSTManager* cstm, char* methodName, genST** interfacesGenerics, size_t interfacesCount);

/* print and free */

void printSymbolTables(CST* cst);
void printNodeSymbolTable(treeNode* n, int indent);
void printClassST(classST* st);
void printMethodST(methodST* st);
void printVarST(varST* st);
void printGenericsST(genST* st);
void printVtable(vtable* vt);
void printCSTM(classSTManager* cstm);

void freeSymbolTables(CST* cst);
void freeNodeSymbolTables(treeNode* n);
void freeClassST(classST* st);
void freeMethodST(methodST* st);
void freeVarST(varST* st);
void freeGenericsST(genST* st);
void freeCSTM(classSTManager* cstm);

void* testcalloc(size_t len, size_t size);

