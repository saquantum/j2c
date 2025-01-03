#pragma once
#include "Object.h"

typedef struct String$obj{
    char* value;
    int valueLength;
    int hash;
    char* className;
    Object$obj* super;
}String$obj;


String$obj* String$String$0(char* value, char* actualType, char* referenceType, char* objName);
    
String$obj* String$String$1(String original, char* actualType, char* referenceType, char* objName);

int String$length$0(String$obj* this);
    
bool String$isEmpty$0(String$obj* this);
    
char String$charAt$0(int index, String$obj* this);

String$obj* String$substring$0(int beginIndex, int endIndex, String$obj* this, char* referenceType, char* objName);
    
bool String$equals$0(String that, String$obj* this);

int String$compareTo$0(String that, String$obj* this);
    
Array$obj* String$toCharArray$0(String$obj* this, char* referenceType, char* objName);
    
int String$hashCode$0(String$obj* this);
    


