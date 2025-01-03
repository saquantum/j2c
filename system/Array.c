#include "Array.h"

struct Array$inner* Array$createLevel(size_t currentDimension, size_t totalDimension, int* sizes){
    if(currentDimension>=totalDimension || sizes[currentDimension]==-1 ){
        return NULL;
    }
    
    struct Array$inner* current = (struct Array$inner*)calloc(1, sizeof(struct Array$inner));
    current->size = sizes[currentDimension];
    if(currentDimension < totalDimension - 1){
        current->inner = (struct Array$inner**)calloc(current->size, sizeof(struct Array$inner*));
        for(size_t i=0; i<current->size; i++){
            current->inner[i] = Array$createLevel(currentDimension+1, totalDimension, sizes);
        }
    }
    return current;
}

struct Array$obj* Array$create(size_t dimension, int* sizes, char* actualType, char* referenceType, char* objName){
    if(dimension==0 || !sizes){
        return NULL;
    }
    
    struct Array$obj* obj = calloc(1, sizeof(struct Array$obj));
    assert(obj);
    
    obj->dimension = dimension;
    if(dimension>=1){
        obj->length = sizes[0];
    }
    obj->super = Object$Object$0(actualType, referenceType, objName);
    
    obj->root= Array$createLevel(0, dimension, sizes);
    assert(obj->root);
    
    return obj;
}

// insert an element. if the entry to be inserted is not initialized, return false
bool Array$insertEntry(struct Array$obj* this, int* indices, void* data){
    assert(this && indices);
    
    struct Array$inner* current = this->root;
    for(size_t i=0; i<this->dimension; i++){
        if(!current){
            return false; 
        }
        if(indices[i]<0 || indices[i]>=(int)current->size){
            return false; // out of bounds
        }
        if(i == this->dimension-1){
            // if current entry has not been occupied, create a box for it
            if(!current->data){
                current->data = calloc(current->size, sizeof(void*));
            }
            current->data[indices[i]] = data;
            return true;
        }
        if(!current->inner || !current->inner[indices[i]]){
            return false; // the subarray is not initialized
        }
        
        current = current->inner[indices[i]];
        
    }
    
    return false;
}

// retrieve a void type pointer, the compiler use the type information stored in Array$obj to cast
void* Array$getEntry(struct Array$obj* this, int* indices){
    assert(this && indices);
    
    struct Array$inner* current = this->root;
    for(size_t i=0; i<this->dimension; i++){
        if(!current){
            return NULL; 
        }
        if(indices[i]<0 || indices[i]>=(int)current->size){
            return NULL; // out of bounds
        }
        if(i == this->dimension-1){
            if(!current->data){
                return NULL;
            }
            return current->data[indices[i]];
        }
        if(!current->inner || !current->inner[indices[i]]){
            return NULL; // the subarray is not initialized
        }
        
        current = current->inner[indices[i]];
        
    }
    
    return NULL;
}

void Array$freeLevel(struct Array$inner* root){
    if(!root){
        return;
    }
    if(root->inner){
        for(size_t i=0; i<root->size; i++){
            Array$freeLevel(root->inner[i]);
        }
        free(root->inner);
    }
    if(root->data){
        free(root->data);
    }
    free(root);
}

void Array$free(struct Array$obj* this){
    if(!this){
        return;
    }
    Array$freeLevel(this->root);
    Object$free(this->super);
    free(this);
}
