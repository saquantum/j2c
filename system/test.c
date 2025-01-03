#include "../headers.h"
#include "Object.h"
#include "String.h"

int main(){
    struct Object$obj* obj = Object$Object$0("Object", "Object", "obj");
    assert(!strcmp(obj->actualType,"Object"));
    assert(!strcmp(obj->referenceType,"Object"));
    assert(!strcmp(obj->objectName,"obj"));
    assert(!strcmp(Object$getName$0(obj,NULL,NULL)->value,"obj"));
    printf("%s\n", Object$toString$0(obj, NULL, NULL)->value);
    
    struct Object$obj* obj2 = Object$Object$0("Object", "Object", "obj2");
    struct Object$obj* obj3 = Object$Object$0("Object", "String", "obj3");
    assert(Object$equals$0(obj, obj));
    assert(Object$equals$0(obj, obj2));
    assert(!Object$equals$0(obj, obj3));
    
    struct String$obj* str = String$String$0("abcdef", "String", "String", "str");
    assert(!strcmp("abcdef", str->value));
    assert(!strcmp(str->super->referenceType, "String"));
    assert(!strcmp(str->super->referenceType,"String"));
    assert(!strcmp(str->super->objectName,"str"));
    struct String$obj* str2 = String$String$1(str, "String", "String", "str2");
    assert(!strcmp("abcdef", str2->value));
    assert(!strcmp(str2->super->referenceType, "String"));
    assert(!strcmp(str2->super->referenceType,"String"));
    assert(!strcmp(str2->super->objectName,"str2"));
    
    assert(String$length$0(str)==6);
    assert(!String$isEmpty$0(str));
    assert(String$charAt$0(0, str)=='a');
    assert(!strcmp(String$substring$0(1, 4, str, NULL, NULL)->value, "bcd"));
    
    struct String$obj* str3 = String$String$0("opq", "String", "String", "str3");
    assert(String$equals$0(str,str2));
    assert(!String$equals$0(str,str3));
    assert(!String$compareTo$0(str,str2));
    assert(String$compareTo$0(str,str3)>0);
    assert(String$hashCode$0(str)==String$hashCode$0(str2));
}
