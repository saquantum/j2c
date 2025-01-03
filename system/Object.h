#pragma once
#include "../headers.h"

struct String$obj;

struct Object$obj{
    char* actualType;
    char* referenceType;
    char* objectName;
    char* address;
};

// Object obj = new Object();
// let compiler send LHS as the refType, RHS as the actualType , obj as objName into the function
// new Object();
// if the object is not received, set objName=NULL
struct Object$obj* Object$Object$0(char* actualType, char* referenceType, char* objectName);

int Object$hashCode$0(struct Object$obj* this);

bool Object$equals$0(struct Object$obj* that, struct Object$obj* this);

struct String$obj* Object$getName$0(struct Object$obj* this, char* referenceType, char* objectName);
    
struct String$obj* Object$getAddress$0(struct Object$obj* this, char* referenceType, char* objectName);
    
struct String$obj* Object$toString$0(struct Object$obj* this, char* referenceType, char* objectName);

