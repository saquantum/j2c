#pragma once
#include "Object.h"

struct Object$obj;

struct Array$inner{
    size_t size; 
    void** data; 
    struct Array$inner** inner; 
};

struct Array$obj{
    size_t dimension; // int[][][] -> dimension=3
    size_t length; // int[5][2][] -> length=5
    struct Array$inner* root; // contains a formal array which must not have data
    struct Object$obj* super; 
};

// all methods below are for the compiler to call, not for user

// initialize undeclared subarray. int[][] arr = new[2][]; arr[0] = new int[10];
// the declaration calls Array$create, assignment calls Array$createLevel. 
struct Array$inner* Array$createLevel(size_t currentDimension, size_t totalDimension, int* sizes);

// before calling this function, compiler must first check the length of sizes equals length
struct Array$obj* Array$create(size_t dimension, int* sizes, char* actualType, char* referenceType, char* objName);

// insert an element 
bool Array$insertEntry(struct Array$obj* this, int* indices, void* data);

// retrieve a void type pointer, the compiler use the type information stored in Array$obj to cast
void* Array$getEntry(struct Array$obj* this, int* indices);

void Array$freeLevel(struct Array$inner* root);

void Array$free(struct Array$obj* this);


