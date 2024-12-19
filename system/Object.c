#include "../headers.h"
#include "Object.h"

Object* Object$Object$0(char* objName){
    Object* obj = calloc(1,sizeof(Object));
    char* tmp0 = "Object";
    obj->actualType = calloc((int)strlen(tmp0)+1, sizeof(char));
    strcpy(obj->actualType, tmp0);
    obj->referenceType = calloc((int)strlen(tmp0)+1, sizeof(char));
    strcpy(obj->referenceType, tmp0);
    obj->objectName = calloc((int)strlen(objName)+1, sizeof(char));
    strcpy(obj->objectName, objName);
    char buffer[256]={0};
    snprintf(buffer, 256, "%p", obj);
    obj->address = calloc((int)strlen(buffer)+1, sizeof(char));
    strcpy(obj->address, buffer);
    return obj;
}

