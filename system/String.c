#include "String.h"

struct String$obj* String$String$0(char* value, char* actualType, char* referenceType, char* objName){
    if(!value){
        fprintf(stderr, "%sNullPointerException: does not support creating String object with null string%s\n", RED, NRM);
        exit(1);
    }
    
    struct String$obj* obj = calloc(1, sizeof(struct String$obj));
    obj->value = mystrdup(value);
    obj->valueLength = (int) strlen(value);
    obj->super = Object$Object$0(actualType, referenceType, objName);
    
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    obj->super->address = mystrdup(buffer);
    
    return obj;
}
    
struct String$obj* String$String$1(struct String$obj* original, char* actualType, char* referenceType, char* objName){

    struct String$obj* obj = calloc(1, sizeof(struct String$obj));
    obj->value = mystrdup(original->value);
    obj->valueLength = (int) strlen(original->value);
    obj->super = Object$Object$0(actualType, referenceType, objName);
    
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    obj->super->address = mystrdup(buffer);
    
    return obj;
}

int String$length$0(struct String$obj* this){
    if(!this){
        return 0;
    }
    return this->valueLength;
}
    
bool String$isEmpty$0(struct String$obj* this){
    if(!this){
        return true;
    }
    return this->valueLength == 0;
}
    
char String$charAt$0(int index, struct String$obj* this){
    if(!this){
        return 0;
    }
    if(index<0 || index>=this->valueLength){
        fprintf(stderr, "%sArrayOutOfBoundException: cannot access String element%s\n", RED, NRM);
        exit(1);
    }
    
    return this->value[index];
}

struct String$obj* String$substring$0(int beginIndex, int endIndex, struct String$obj* this, char* referenceType, char* objName){
    if(beginIndex>endIndex || beginIndex<0 || endIndex>this->valueLength){
        fprintf(stderr, "%sArrayOutOfBoundException: cannot access String element%s\n", RED, NRM);
        exit(1);
    }
    int len = endIndex - beginIndex;
    char* tmp = calloc(len+1, sizeof(char));
    for(int i=beginIndex; i<endIndex; i++){
        tmp[i-beginIndex] = this->value[i];
    }
    return String$String$0(tmp, "String", referenceType, objName);
}


bool String$equals$0(struct String$obj* that, struct String$obj* this){
    if(!this && !that){
        return true;
    }
    if(!this || !that){
        return false;
    }
    if(that->valueLength != this->valueLength){
        return false;
    }
    if(strcmp(this->value, that->value)){
        return false;
    }
    return true;
}

int String$compareTo$0(struct String$obj* that, struct String$obj* this){
    if(!this || !that){
        return 0;
    }
    // the order cannot be changed, since strcmp(that, this) gives the contrary result
    return strcmp(this->value, that->value);
}

int String$hashCode$0(struct String$obj* this){
    int hash = hash_djb2(this->value);
    this->hash = hash;
    return hash;
}
    
/*
struct Array$obj* String$toCharArray$0(struct String$obj* this, char* referenceType, char* objName){
    
}
*/


