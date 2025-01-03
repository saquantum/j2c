#pragma once
#include "../headers.h"

typedef struct Object$obj{
    char* actualType;
    char* referenceType;
    char* className;
    char* objectName;
    char* address;
}Object$obj;

// Object obj = new Object();
// let compiler send LHS as the refType, RHS as the actualType , obj as objName into the function
// new Object();
// if the object is not received, set objName=NULL
Object$obj* Object$Object$0(char* actualType, char* referenceType, char* objName);

int Object$hashCode$0(Object$obj* this);

bool Object$equals$0(Object$obj* that, Object$obj* this);

String$obj* Object$getName$0(Object$obj* this);
    
String$obj* Object$getAddress$0(Object$obj* this);
    
String$obj* Object$toString$0(Object$obj* this);
