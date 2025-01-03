#pragma once
#include "Object.h"

typedef struct Array$inner{
    size_t size; // size of current array.
    void* data; // if current array is an entry, then inner=NULL, size=0
    Array$inner** inner; // current array contains subarray, then data=NULL
}Array$inner;

typedef struct Array$obj{
    char* type; // for casting. String -> String$obj*
    size_t dimension; // int[][][] -> dimension=3
    Array$inner** root; // contains a formal array which must not have data
    Object$obj* super; 
}Array$obj;

// all methods below are for compiler to call, not for user

Array$obj* Array$create(size_t dimension, int* sizes);

// initialize undeclared subarray. int[][] arr = new[2][]; arr[0] = new int[10];
// the declaration calls Array$create, assignment calls Array$initSubarray. 
Array$inner* Array$initSubarray();

// insert an element 
bool Array$insertEntry(Array$obj* this, int* indices, void* data);

// retrieve a void type pointer, the compiler use the type information stored in Array$obj to cast
void* Array$getEntry(Array$obj* this, int* indices);
