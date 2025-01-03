#include "String.h"

String$obj* String$String$0(char* value, char* actualType, char* referenceType, char* objName){
    if(!value){
        fprintf(stderr, "%sNullPointerException: does not support creating String object with null string%s\n", RED, NRM);
        exit(1);
    }
    
    String$obj* obj = calloc(1, sizeof(String$obj));
    obj->value = mystrdup(value);
    obj->valueLength = (int) strlen(value);
    obj->className = mystrdup("String");
    obj->super = Object$Object$0(actualType, referenceType, objName);
    
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    void* tmp = obj->super;
    while(strcmp(tmp->className, "Object")){
        tmp = (void*) tmp->super;
    }
    tmp->address = mystrdup(buffer);
    
    return obj;
}
    
String$obj* String$String$1(String$obj* original, char* actualType, char* referenceType, char* objName){

    String$obj* obj = calloc(1, sizeof(String$obj));
    obj->value = mystrdup(original->value);
    obj->valueLength = (int) strlen(original->value);
    obj->className = mystrdup("String");
    obj->super = original->super;
    
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    void* tmp = obj->super;
    while(strcmp(tmp->className, "Object")){
        tmp = (void*) tmp->super;
    }
    tmp->address = mystrdup(buffer);
    
    return obj;
}

int String$length$0(String$obj* this){
    if(!this){
        return;
    }
    return this->valueLength;
}
    
bool String$isEmpty$0(String$obj* this){
    if(!this){
        return;
    }
    return this.valueLength == 0;
}
    
char String$charAt$0(int index, String$obj* this){
    if(!this){
        return;
    }
    if(index<0 || index>=this->valueLength){
        fprintf(stderr, "%sArrayOutOfBoundException: cannot access String element%s\n", RED, NRM);
        exit(1);
    }
    
    return this->value[index];
}

String$obj* String$substring$0(int beginIndex, int endIndex, String$obj* this, char* referenceType, char* objName){
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

bool String$equals$0(String that, String$obj* this){
    
}

int String$compareTo$0(String that, String$obj* this){
    
}
    
Array$obj* String$toCharArray$0(String$obj* this, char* referenceType, char* objName){
    
}
int String$hashCode$0(String$obj* this){
    int hash = hash_djb2(this->value);
    this->hash = hash;
    return hash;
}
