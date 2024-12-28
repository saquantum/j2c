#pragma once
#include "parser.h"

typedef enum classification_of_ST{
    CLASS_ST,
    METHOD_ST,
    VAR_ST,
    GEN_ST
} classification_of_ST;

typedef struct classST{
    classification_of_ST cf;
    char* name;
    
    genST* generics;
    
    char* superclass;
    genST* superclassGenerics;
    char** interfaces;
    genST** interfacesGenerics;
    
    varST** fields;
    
    methodST** methods;
    
    bool isPublic;
    bool isPrivate;
    bool isAbstract;
    bool isStatic;
    bool isFinal;
} classST;

typedef struct methodST{
    classification_of_ST cf; // should be set to METHOD_ST by default
    char* name;
    
    genST* generics; // type boundedness for this method, this affects arguments
    
    varST** arguments; // array of arguments of this method, NULL -> no argument
    
    varST** locals; // array of local variables, NULL -> no local
    
}methodST;

// Map<? extends Comparable<?> , V extends List<String> >
typedef struct genST{
    classification_of_ST cf; // should be set to GEN_ST by default
    char* name; // the identifier before extends or super. Map< ... , ... > -> name = NULL
    bool extends;
    bool super;
    char* type; // Map is the type!
    struct genST** nested; // to next level. {name="?", extends=true, super=false, type=Comparable, nested={name=NULL, extends=false, super=false, type="?"}}, {name="V", extends=true, super=false, type=List, nested={name=NULL, extends=false, super=false, type="String"}}.
}genST;

typedef struct varST{
    classification_of_ST cf; // should be set to VAR_ST by default
    char* name;
    genST* type; // type of a variable is with generics by default
    
    // array dimension determines number of asterisks for a pointer
    int arrDimension; // if 0, not an array
    int* arrSizes;
    
    bool isPublic;
    bool isPrivate;
    bool isStatic;
    bool isFinal;
}varST;
