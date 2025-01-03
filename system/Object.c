#include "Object.h"

Object$obj* Object$Object$0(char* actualType, char* referenceType, char* objName){
    if(!actualType || !referenceType){
        return NULL;
    }
    Object$obj* obj = calloc(1,sizeof(Object));
    obj->actualType = mystrdup(actualType);
    obj->referenceType = mystrdup(referenceType);
    obj->className = mystrdup("Object");
    obj->objectName = objName?mystrdup(objName):NULL;
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    obj->address = mystrdup(buffer);
    return obj;
}

int Object$hashCode$0(Object$obj* this){
    if(!this){
        return 0;
    }
    int hash = hash_djb2(this->actualType);
    hash = 31*hash + hash_djb2(this->referenceType);
    hash = 31*hash + hash_djb2(this->objName);
    return hash;
}

bool Object$equals$0(Object$obj* that, Object$obj* this){
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

String$obj* Object$getName$0(Object$obj* this, char* referenceType, char* objName){
    return String$String$0(this->objectName, "String", referenceType, objName);
}
    
String$obj* Object$getAddress$0(Object$obj* this, char* referenceType, char* objName){
    return String$String$0(this->address, "String", referenceType, objName);
}
    
String$obj* Object$toString$0(Object$obj* this, char* referenceType, char* objName){
    size_t len = strlen(this->actualType) + strlen(this->referenceType) + strlen(this->objName);
    char* tmp = calloc((int)len+5, sizeof(char));
    strcpy(tmp, this->actualType);
    strcpy(tmp, ", ");
    strcpy(tmp, this->referenceType);
    strcpy(tmp, ", ");
    strcpy(tmp, this->objName);
    return String$String$0(tmp, "String", referenceType, objName);
}
