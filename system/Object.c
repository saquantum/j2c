#include "Object.h"
#include "String.h"

struct Object$obj* Object$Object$0(char* actualType, char* referenceType, char* objectName){
    if(!actualType){
        return NULL;
    }
    struct Object$obj* obj = calloc(1,sizeof(struct Object$obj));
    obj->actualType = mystrdup(actualType);
    obj->referenceType = mystrdup(referenceType);
    obj->objectName = objectName?mystrdup(objectName):NULL;
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    obj->address = mystrdup(buffer);
    return obj;
}

int Object$hashCode$0(struct Object$obj* this){
    if(!this){
        return 0;
    }
    int hash = hash_djb2(this->actualType);
    hash = 31*hash + hash_djb2(this->referenceType);
    hash = 31*hash + hash_djb2(this->objectName);
    return hash;
}

bool Object$equals$0(struct Object$obj* that, struct Object$obj* this){
    if(!that && !this){
        return true;
    }
    if(!that && this){
        return false;
    }
    if(that && !this){
        return false;
    }
    if(!strcmp(that->address, this->address)){
        return true;
    }
    // all other fields are the same except name
    return !strcmp(that->actualType, this->actualType) && !strcmp(that->referenceType, this->referenceType);
}


struct String$obj* Object$getName$0(struct Object$obj* this, char* referenceType, char* objectName){
    return String$String$0(this->objectName, "String", referenceType, objectName);
}
    
struct String$obj* Object$getAddress$0(struct Object$obj* this, char* referenceType, char* objectName){
    return String$String$0(this->address, "String", referenceType, objectName);
}
    
struct String$obj* Object$toString$0(struct Object$obj* this, char* referenceType, char* objectName){
    size_t len = strlen(this->actualType) + strlen(this->referenceType) + strlen(this->objectName);
    char* tmp = calloc((int)len+43, sizeof(char));
    strcat(tmp, "name = ");
    strcat(tmp, this->objectName);
    strcat(tmp, ", actual type = ");
    strcat(tmp, this->actualType);
    strcat(tmp, ", reference type = ");
    strcat(tmp, this->referenceType);
    return String$String$0(tmp, "String", referenceType, objectName);
}

