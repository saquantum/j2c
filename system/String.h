#pragma once
#include "Object.h"

struct Object$obj;
struct Array$obj;

struct String$obj{
    char* value;
    int valueLength;
    int hash;
    struct Object$obj* super;
};

struct String$obj* String$String$0(char* value, char* actualType, char* referenceType, char* objName);
    
struct String$obj* String$String$1(struct String$obj* original, char* actualType, char* referenceType, char* objName);

int String$length$0(struct String$obj* this);
    
bool String$isEmpty$0(struct String$obj* this);
    
char String$charAt$0(int index, struct String$obj* this);

struct String$obj* String$substring$0(int beginIndex, int endIndex, struct String$obj* this, char* referenceType, char* objName);

bool String$equals$0(struct String$obj* that, struct String$obj* this);

int String$compareTo$0(struct String$obj* that, struct String$obj* this);

int String$hashCode$0(struct String$obj* this);
    
struct Array$obj* String$toCharArray$0(struct String$obj* this, char* referenceType, char* objName);

void String$free(struct String$obj* this);


    


