#include "semantics.h"

bool checkCSTM(classSTManager* cstm){
    if(!cstm){
        return false;
    }
    if(!cstm->hasMain){
        fprintf(stderr, "%sWarning checkClassInheritance: program does not have main function%s\n", RED, NRM);
    }
    checkDuplicateClass(cstm);
    for(size_t i=0; i < cstm->length; i++){
        classST* st = cstm->registeredTables[i];
        if(!checkClass(cstm, st)){
            return false;
        }
    }
    return true;
}

bool checkDuplicateClass(classSTManager* cstm){
    if(!cstm){
        return true;
    }
    // check no duplicate name is used for classes and interfaces
    for(size_t i=0; i < cstm->length; i++){
        for(size_t j=0; j < cstm->length; j++){
            if(i!=j && !strcmp(cstm->registeredTables[i]->generics->type, cstm->registeredTables[j]->generics->type)){
                fprintf(stderr, "%sError checkDuplicateClass: class/interface identifier %s is duplicate%s\n", RED, cstm->registeredTables[i]->generics->type, NRM);
                exit(1);
            }
        }
    }
    return true;
}

bool checkClass(classSTManager* cstm, classST* st){
    if(!cstm || !st){
        return false;
    }
    if(st->isClass && !checkClassInheritance(cstm, st)){
        return false;
    }
    if(!checkInterfaceImplementation(cstm, st) || !checkAbstract(cstm, st)){
        return false;
    }
    if(!checkMethods(cstm, st)){
        return false;
    }
       
    return true;
}

bool checkClassInheritance(classSTManager* cstm, classST* st){
    if(!cstm || !st || !st->isClass){
        return false;
    }
    // 3 tasks: check superclass exists, superclass not final, no cyclic extension
    // but first, if it is Object, we can save all effort.
    if(!strcmp(st->generics->type, "Object")){
        return true;
    }
    // check superclass existence
    classST* super = lookupClassST(cstm, st->superclassGenerics->type);
    if(!super){
        fprintf(stderr, "%sError checkClassInheritance: superclass %s of %s does not exist%s\n", RED, st->superclassGenerics->type, st->generics->type, NRM);
        exit(1);
    }
    // check superclass is a class
    if(!super->isClass){
        fprintf(stderr, "%sError checkClassInheritance: superclass %s of %s is not a valid class%s\n", RED, st->superclassGenerics->type, st->generics->type, NRM);
        exit(1);
    }
    // check superclass not final
    if(super->isFinal){
        fprintf(stderr, "%sError checkClassInheritance: final class %s cannot be extended by %s%s\n", RED, st->superclassGenerics->type, st->generics->type, NRM);
        exit(1);
    }
    // check cyclic extension
    while(super){
        // Object does not extend Object, so this is safe
        if(!strcmp(super->generics->type, "Object")){
            break;
        }
        // if some super-superclass extended current class, wrong
        if(!strcmp(st->generics->type, super->generics->type)){
            fprintf(stderr, "%sError checkClassInheritance: cyclic extension detected %s%s\n", RED, st->generics->type, NRM);
            exit(1);
        }
        super = lookupClassST(cstm, super->superclassGenerics->type);
    }
    
    return true;
}

bool checkInterfaceImplementation(classSTManager* cstm, classST* st){
    if(!cstm || !st){
        return false;
    }
    if(!st->interfacesCount){
        return true;
    }
    
    // this function applies to both class and interface.
    // 3 tasks: check interfaces exist, check all methods are implemented, check cyclic implementation.
    
    // check all interfaces exists and are interfaces
    for(size_t i=0; i < st->interfacesCount; i++){
        classST* super = lookupClassST(cstm, st->interfacesGenerics[i]->type);
        if(!super){
            fprintf(stderr, "%sError checkInterfaceImplementation: interface %s implemented by %s does not exist%s\n", RED, st->interfacesGenerics[i]->type, st->generics->type, NRM);
            exit(1);
        }
        if(!super->isInterface){
            fprintf(stderr, "%sError checkInterfaceImplementation: interface %s implemented by %s is not an interface%s\n", RED, st->interfacesGenerics[i]->type, st->generics->type, NRM);
            exit(1);
        }
        // check cyclic implementation
        if(!checkCyclicImplementation(cstm, st, super)){
            fprintf(stderr, "%sError checkInterfaceImplementation: cyclic implemetation of interfaces detected %s%s\n", RED, st->generics->type, NRM);
            exit(1);
        }
    }
    
    if(st->isInterface){
        return true;
    }
    
    // st is class, check if all methods have been implemented
    for(size_t i=0; i < st->interfacesCount; i++){
        classST* super = lookupClassST(cstm, st->interfacesGenerics[i]->type);
        for(size_t j=0; j < super->virtualTable->entryCount; j++){
            bool found = false;
            for(size_t k=0; k < st->methodsCount; k++){
                printf("Debug: check method %s from %s for %s\n", st->methods[k]->generics->type, st->generics->type, super->virtualTable->entries[j]->generics->type);
                if(methodOverrides(st->methods[k], super->virtualTable->entries[j])){  
                if(isValidOverrideReturnType(st->methods[k], super->virtualTable->entries[j], cstm)
                ){
                    if(st->methods[k]->isStatic){
                        fprintf(stderr, "%sError checkInterfaceImplementation: method %s implemented from interface cannot be static%s\n", RED, st->methods[k]->generics->type, NRM);
                        exit(1);
                    }
                    printf("Debug: overrides\n");
                    found = true;
                }}
            }
            if(!found){
                fprintf(stderr, "%sError checkInterfaceImplementation: valid implementation in %s of abstract method %s from interface %s not found%s\n", RED, st->generics->type, super->virtualTable->entries[j]->generics->type, st->interfacesGenerics[i]->type, NRM);
                exit(1);
            }
        }
    }
    

    return true;
}

bool checkCyclicImplementation(classSTManager* cstm, classST* st, classST* current){
    if(!cstm || !st || !current){
        return true;
    }
    for(size_t i=0; i < current->interfacesCount; i++){
        classST* super = lookupClassST(cstm, current->interfacesGenerics[i]->type);
        if(!strcmp(super->generics->type, st->generics->type)){
            return false;
        }
        if(!checkCyclicImplementation(cstm, st, super)){
            return false;
        }
    }
    return true;
}

bool checkAbstract(classSTManager* cstm,classST* st){
    if(!cstm || !st){
        return false;
    }
    
    // 3 cases: non-abstract class, abstract class, interface
    if(st->isInterface){
        if(st->isAbstract){
            fprintf(stderr, "%sWarning checkAbstract: interface %s does not need an abstract modifier\n%s", RED, st->generics->type, NRM);
        }
        for(size_t i=0; i < st->methodsCount; i++){
            if(st->methods[i]->isAbstract){
                fprintf(stderr, "%sError checkAbstract: method %s does not need an abstract modifier in interface %s%s\n", RED, st->methods[i]->generics->type, st->generics->type, NRM);
                exit(1);
            }
            if(!isEmptyMethod(st->methods[i])){
                fprintf(stderr, "%sError checkAbstract: method %s should have an empty body in interface %s%s\n", RED, st->methods[i]->generics->type, st->generics->type, NRM);
                exit(1);
            }
        }
        return true;
    }
    
    if(st->isClass && !st->isAbstract){
        for(size_t i=0; i < st->methodsCount; i++){
            if(st->methods[i]->isAbstract){
                fprintf(stderr, "%sError checkAbstract: abstract method %s not allowed in concrete class %s%s\n", RED, st->methods[i]->generics->type, st->generics->type, NRM);
                exit(1);
            }
        }
        return true;
    }
    
    for(size_t i=0; i < st->methodsCount; i++){
        if(!st->methods[i]->isAbstract){
            fprintf(stderr, "%sError checkAbstract: non-abstract method %s not allowed in abstract class%s%s\n", RED, st->methods[i]->generics->type, st->generics->type, NRM);
            exit(1);
        }
        if(!isEmptyMethod(st->methods[i])){
            fprintf(stderr, "%sError checkAbstract: method %s should have an empty body in abstract class %s%s\n", RED, st->methods[i]->generics->type, st->generics->type, NRM);
            exit(1);
        }
        if(!isEmptyMethod(st->methods[i])){
            fprintf(stderr, "%sError checkAbstract: method %s should have an empty body in abstract class %s%s\n", RED, st->methods[i]->generics->type, st->generics->type, NRM);
            exit(1);
        }
    }
    return true;
}

bool checkMethods(classSTManager* cstm, classST* st){
    if(!cstm || !st){
        return true;
    }
    // check if two methods can override but belong to one class
    for(size_t i=0; i < st->methodsCount; i++){
        // extra for an interface, check no constructor presents
        if(st->isInterface && st->methods[i]->isConstructor){
            fprintf(stderr, "%sError checkMethods: no constructor allowed in interface %s\n%s", RED, st->generics->type, NRM);
            exit(1);
        }
        // check constructor has identical name with class
        if(st->methods[i]->isConstructor && strcmp(st->methods[i]->generics->type, st->generics->type)){
            fprintf(stderr, "%sError checkMethods: identifier of constructor %s is to identical to that of the class %s\n%s", RED, st->methods[i]->generics->type, st->generics->type, NRM);
            exit(1);
        }
        // for static class, all methods must be static
        if(st->isStatic && !st->methods[i]->isStatic){
            fprintf(stderr, "%sError checkMethods: method %s is not static in a static class %s\n%s", RED, st->methods[i]->generics->type, st->generics->type, NRM);
            exit(1);
        }
        // check no duplicate argument identifier
        if(!checkDuplicateArgument(st->methods[i])){
            fprintf(stderr, "%sError checkMethods: method %s in class %s has arguments with identical name\n%s", RED, st->methods[i]->generics->type, st->generics->type, NRM);
            exit(1);
        }
        // check method details
        if(!checkMethodTree(cstm, st->methods[i])){
            fprintf(stderr, "%sError checkMethods: empty or wild pointer has been sent in for method %s%s\n", RED, st->methods[i]->generics->type, NRM);
            exit(1);
        }
        // check no duplicate method
        for(size_t j=0; j < st->methodsCount; j++){
            if(i!=j && methodOverrides(st->methods[i], st->methods[j])){
                fprintf(stderr, "%sError checkMethods: duplicate method %s in %s\n%s", RED, st->methods[i]->generics->type, st->generics->type, NRM);
                exit(1);
            }
        }
    }
    // check no field has duplicate identifier
    for(size_t i=0; i < st->fieldsCount; i++){
        // no non-static variable for a static class
        if(st->isStatic && !st->fields[i]->isStatic){
            fprintf(stderr, "%sError checkMethods: field %s is not static in a static class %s\n%s", RED, st->fields[i]->name, st->generics->type, NRM);
            exit(1);
        }
        for(size_t j=0; j < st->fieldsCount; j++){
            if(i!=j && !strcmp(st->fields[i]->name, st->fields[j]->name)){
                fprintf(stderr, "%sError checkMethods: duplicate variable identifier %s in %s\n%s", RED, st->fields[i]->name, st->generics->type, NRM);
                exit(1);
            }
        }
    }
    return true;
}

bool checkDuplicateArgument(methodST* st){
    if(!st){
        return true;
    }
    // check if arguments have duplicate identifier
    for(size_t i=0; i < st->argumentsCount; i++){
        for(size_t j=0; j < st->argumentsCount; j++){
            if(i!=j && !strcmp(st->arguments[i]->name, st->arguments[j]->name)){
                fprintf(stderr, "%sError checkDuplicateArgument: duplicate argument identifier %s in %s\n%s", RED, st->arguments[i]->name, st->generics->type, NRM);
                exit(1);
            }
        }
    }
    return true;
}

bool isEmptyMethod(methodST* st){
    if(!st){
        return true;
    }
    if(st->attachNode->ruleType == subroutinePrototype_rule){
        return true;
    }
    treeNode* node = st->attachNode;
    if(!node){
        fprintf(stderr, "%sError isEmptyMethod: unexpected empty attach node for method %s%s\n", RED, st->generics->type, NRM);
        exit(1);
    }
    treeNode* body = NULL;
    for(size_t i=0; i < node->childCount; i++){
        if(node->children[i]->ruleType == subroutineBody_rule){
            body = node->children[i];
        }
    }
    assert(body);
    return !(body->childCount > 0);
}

bool checkMethodTree(classSTManager* cstm, methodST* st){
    if(!cstm || !st){
        return false;
    }
    if(!checkMethodTreeHelper(cstm, st->attachNode)){
        return false;
    }
    return true;
}

bool checkMethodTreeHelper(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return true; // returns true so one can recursively call this
    }
    // this function, should check all details in a method
    if(n->ruleType == variableDeclaration_rule){
        if(!checkVariableDeclaration(cstm, n)){
            return false;
        }
    }
    if(n->ruleType == statement_rule){
        if(!checkStatement(cstm, n)){
            return false;
        }
    }
    for(size_t i=0; i < n->childCount; i++){
        if(!checkMethodTreeHelper(cstm, n->children[i])){
            return false;
        }
    }
    return true;
}


bool checkVariableDeclaration(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return false;
    }
    if(n->ruleType != variableDeclaration_rule){
        return false;
    }
    // check reference type exists
    if(!n->varSymbolTable[0]->isPrimitive){
        classST* tmp = lookupClassST(cstm, n->varSymbolTable[0]->type->type);
        if(!tmp){
            fprintf(stderr, "%sError checkVariableDeclaration: reference type %s of variable %s does not exist%s\n", RED, n->varSymbolTable[0]->type->type, n->varSymbolTable[0]->name, NRM);
            exit(1);
        }
    }
    // check assignment
    for(size_t i=0; i < n->childCount; i++){
        if(n->children[i]->ruleType == assignment_rule && !checkAssignmentCompatible(cstm, n->children[i])){
            fprintf(stderr, "%sError checkVariableDeclaration: invalid assignment%s\n", RED, NRM);
            exit(1);
        }
    }
    return true;
}

int countArrayDimension(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return -1;
    }
    if(n->ruleType != arrayInitialization_rule){
        return -1;
    }
    int max = 1;
    int tmp = 1;
    bool firstEntry = true;
    
    for(size_t i=0; i < n->childCount; i++){
        if(n->children[i]->ruleType != arrayInitialization_rule){
            tmp = countArrayDimension(cstm, n->children[i]);
            if(tmp != max && !firstEntry){
                fprintf(stderr, "%sError countArrayDimension line %d: invalid array initialization%s\n", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            max = tmp;
            firstEntry = false;
        }
    }
    return max;
}

bool checkAssignmentCompatible(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return false;
    }
    if(n->ruleType != assignment_rule){
        return false;
    }
    // if RHS is array init, count its dimension
    
    
    // check LHS and RHS of assignment has compatible type
    int typeLHS = typeOfTerm(cstm, n->children[0]);
    int typeRHS = typeOfExpression(cstm, n->children[2]);
    if(-5 <= typeLHS && typeLHS <= -2 && -5 <= typeRHS && typeRHS <= -2){
        return true;
    }
    if(typeLHS >= 0 && typeRHS >= 0 && isvalidInheritance(cstm, typeLHS, typeRHS)){
        return true;
    }
    printf("typeLHS = %d, typeRHS = %d, line number = %d\n", typeLHS, typeRHS, n->children[1]->assoToken->lineNumber);
    return typeLHS == typeRHS;
}

int typeOfTerm(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return -1;
    }
    // this function, returns the type of the term, referring to the cstm, 
    // for a reference type in the cstm, return a positive number 
    // referring to its index in the registered array.
    // for primitive types, negative numbers are used.
    // -2: char -3: int -4: long -5: double
    // -10: boolean
    // -11: string literal
    // -100: null
    // return -1 if any error.
    
    if(n->ruleType != term_rule || !n->childCount){
        return -1;
    }
    
    // find method declaration or class declaration current term belongs to
    treeNode* parentMethod = NULL;
    treeNode* parentClass = NULL;
    treeNode* tmp = n->parent;
    // interface declaration is excluded since in my version
    // interface can only have empty method prototype
    while(tmp->ruleType != classDeclaration_rule){
        // parent method is optional for class fields
        if(tmp->ruleType == subroutineDeclaration_rule){
            parentMethod = tmp;
        }
        tmp = tmp->parent;
    }
    parentClass = tmp;
    assert(parentClass);
    
    if(n->childCount == 1){
        treeNode* current = n->children[0];
        // parenthesized expression
        if(current->ruleType == parenthesizedExpression_rule){
            return typeOfExpression(cstm, current->children[1]);
        }
        // terminal terms (null, true, false, this, super)
        if(current->ruleType == terminalTerm_rule){
            if(current->assoToken->data.key_val == NULLER){
                return -100;
            }
            if(current->assoToken->data.key_val == BOOL_TRUE ||
               current->assoToken->data.key_val == BOOL_FALSE){
                return -10;    
            }
            if(current->assoToken->data.key_val == THIS){
                return lookupIndexClassST(cstm, parentClass->classSymbolTable->generics->type);
            }
            if(tmp->assoToken->data.key_val == SUPER){
                return lookupIndexClassST(cstm, parentClass->classSymbolTable->superclassGenerics->type);
            }
        }
        // number is set to double for simplicity
        if(current->ruleType == number_rule){
            return -5;
        }
        // char
        if(current->ruleType == character_rule){
            return -2;
        }
        // string literal
        if(current->ruleType == string_rule){
            return -11;
        }
        // identifier follows java's scope resolution rule
        if(current->ruleType == identifier_rule){
           return typeOfIdentifier(cstm, current, parentMethod, parentClass);
        }
        // new object
        if(current->ruleType == newObject_rule){
            assert(current->children[1]->ruleType == type_rule);
            assert(current->children[1]->childCount == 1);
            if(current->children[1]->children[0]->ruleType == primitiveType_rule){
                return -1;
            }
            // newObject->type->referenceType->identifier
            return typeOfIdentifier(cstm, current->children[1]->children[0]->children[0], parentMethod, parentClass);
        }
        // array init
        if(current->ruleType == arrayInitialization_rule){
            // ensure all entries are of the same type
            size_t idx = 1;
            int type = -1;
            bool firstEntry = true;
            while(idx < current->childCount){
                int tmp = -1;
                if(current->children[idx]->ruleType == term_rule){
                    tmp = typeOfTerm(cstm, current->children[idx]);
                }
                if(current->children[idx]->ruleType == expression_rule){
                    tmp = typeOfExpression(cstm, current->children[idx]);
                }
                if(firstEntry){
                    firstEntry = false;
                }else if(type != tmp){
                    return -1;
                }
                type = tmp;
                idx += 2;
            }
            return type;
        }
        // method call
        if(current->ruleType == subroutineCall_rule){
            // the method must be from current class, or in the vtable
            classST* st = parentClass->classSymbolTable;
            vtable* vt = st->virtualTable;
            for(size_t i=0; i < st->methodsCount; i++){
                if(!strcmp(st->methods[i]->generics->type, current->assoToken->data.str_val)){
                    // the method could either be a constructor, returns number, returns bool
                    // or returns other reference type
                    
                    // constructor
                    if(st->methods[i]->isConstructor){
                        return lookupIndexClassST(cstm, parentClass->classSymbolTable->generics->type);
                    }
                    // number
                    if(st->methods[i]->returnsNumber){
                        return -5;
                    }
                    // bool
                    if(st->methods[i]->returnsBool){
                        return -10;
                    }
                    return lookupIndexClassST(cstm, st->methods[i]->returnType->type);
                }
            }
            for(size_t i=0; i < vt->entryCount; i++){
                if(!strcmp(vt->entries[i]->generics->type, current->assoToken->data.str_val)){
                    // number
                    if(vt->entries[i]->returnsNumber){
                        return -5;
                    }
                    // bool
                    if(vt->entries[i]->returnsBool){
                        return -10;
                    }
                    return lookupIndexClassST(cstm, vt->entries[i]->returnType->type);
                }
            }
            
            return -1;
        }
    }
    
    return -1;
}

int typeOfIdentifier(classSTManager* cstm, treeNode* n, treeNode* parentMethod, treeNode* parentClass){
    if(!cstm || !n || !parentClass){
        return -1;
    }
    if(n->ruleType != identifier_rule){
        return -1;
    }
            int idx = -1;
            // a local variable in current method
            if(parentMethod){
                idx = lookupVarInMethod(parentMethod->methodSymbolTable, n->assoToken->data.str_val);
                if(idx != -1){
                    return lookupIndexClassST(cstm, parentMethod->methodSymbolTable->locals[idx]->type->type);
                }
            }
            // a field in current class
            idx = lookupVarInClass(parentClass->classSymbolTable, n->assoToken->data.str_val);
            if(idx != -1){
                return lookupIndexClassST(cstm, parentClass->classSymbolTable->fields[idx]->type->type);
            }
            // if current class is Object, the identifier is undefined
            if(!strcmp("Object", parentClass->classSymbolTable->generics->type)){
                return -1;
            }
            // a field from super class, or undefined variable
            classST* super = lookupClassST(cstm, parentClass->classSymbolTable->superclassGenerics->type);
            return lookupVarInClass(super, n->assoToken->data.str_val);
        
}

int typeOfExpression(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return -1;
    }
    // ternary
    if(n->ruleType == ternaryExpression_rule){
        // test whether it has ternary operators.
        if(n->childCount == 1){
            // in this case the logical or expression can be of any type.
            return typeOfExpression(cstm, n->children[0]);
        }else{
            // in this case the logical or expression can only be boolean
            if(typeOfExpression(cstm, n->children[0]) != -10){
                fprintf(stderr, "%sError typeOfExpression line %d: the first part of ternary expression is not boolean\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }

            int t1 = typeOfExpression(cstm, n->children[2]);
            int t2 = typeOfExpression(cstm, n->children[4]);
            if(t1 != t2){
                printf("%s Error typeOfExpression: for simplicity of code, the two branches of the ternary expression at line %d must not be of defferent types\n%s", RED, n->assoToken->lineNumber, NRM);
            }
            return t1;
        }
    }
    // logical or, and
    if(n->ruleType == logicalOrExpression_rule || n->ruleType == logicalAndExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        }else{
            for(size_t i=0; i < n->childCount; i+=2){
                if(typeOfExpression(cstm, n->children[i]) != -10){
                    fprintf(stderr, "%sError typeOfExpression line %d: logical operator combines non-boolean expressions.\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
                }
                return -10;
            }
        }
    }
    // bitwise or, and, xor
    if(n->ruleType == bitwiseOrExpression_rule || n->ruleType == bitwiseXorExpression_rule || n->ruleType == bitwiseAndExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        }else{
            int type = -2;
            for(size_t i=0; i < n->childCount; i+=2){
                int tmp = typeOfExpression(cstm, n->children[i]);
                if( tmp <=-5 || tmp >=-2 ){
                    fprintf(stderr, "%sError typeOfExpression line %d: bitwise operator combines non-number expressions.\n%s", RED, n->assoToken->lineNumber, NRM);
                    exit(1);
                }
                type = tmp<type ? tmp : type;
            }
            return type;
        }
    }
    // equality 
    if(n->ruleType == equalityExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        }else{
            int type = -2;
            bool firstEntry = true;
            for(size_t i=0; i < n->childCount; i+=2){
                int tmp = typeOfExpression(cstm, n->children[i]);
                if( tmp <=-10 || tmp >=-2){
                    fprintf(stderr, "%sError typeOfExpression line %d: compares non-number and non-boolean expressions.\n%s", RED, n->assoToken->lineNumber, NRM);
                    exit(1);
                }
                if((tmp==-10 || type==-10) && tmp!=type && !firstEntry){
                    fprintf(stderr, "%sError typeOfExpression line %d: compares a number and a boolean expressions in one expression.\n%s", RED, n->assoToken->lineNumber, NRM);
                    exit(1);
                }
                firstEntry = false;
                type = tmp<type ? tmp : type;
            }
            return type;
        }
    }
    // relational 
    if(n->ruleType == relationalExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        }else if(n->children[1]->ruleType != instanceof_rule){
            int type = -2;
            for(size_t i=0; i < n->childCount; i+=2){
                int tmp = typeOfExpression(cstm, n->children[i]);
                if( tmp <=-5 || tmp >=-2){
                    fprintf(stderr, "%sError typeOfExpression line %d: compares non-number expressions.\n%s", RED, n->assoToken->lineNumber, NRM);
                    exit(1);
                }
                type = tmp<type ? tmp : type;
            }
            return type;
        // instanceof
        }else{
            int type0 = typeOfExpression(cstm, n->children[0]);
            if(type0 < 0){
                fprintf(stderr, "%sError typeOfExpression line %d: cannot test primitive type using instanceof\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            int type2 = typeOfExpression(cstm, n->children[2]);
            if(type2 < 0){
                fprintf(stderr, "%sError typeOfExpression line %d: invalid reference type instanceof clause\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            return -10;
        }
    }
    // shift, additive, multiplicative
    if(n->ruleType == shiftExpression_rule || n->ruleType == additiveExpression_rule || n->ruleType == multiplicativeExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        }else{
            int type = -2;
            for(size_t i=0; i < n->childCount; i+=2){
                int tmp = typeOfExpression(cstm, n->children[i]);
                if( tmp <=-5 || tmp >=-2 ){
                    fprintf(stderr, "%sError typeOfExpression line %d: arithmetic operator combines non-number expressions.\n%s", RED, n->assoToken->lineNumber, NRM);
                    exit(1);
                }
                type = tmp<type ? tmp : type;
            }
            return type;
        }
    }
    // cast 
    if(n->ruleType == castExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        }else{
            int type1 = typeOfExpression(cstm, n->children[1]);
            int type3 = typeOfExpression(cstm, n->children[3]);
            if(type1 < 0 && type3 >= 0){
                fprintf(stderr, "%sError typeOfExpression line %d: cannot cast an reference type object into primitive type\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            if(type1 >= 0 && type3 < 0){
                fprintf(stderr, "%sError typeOfExpression line %d: cannot cast an primitive type expression into reference type\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            return type1;
        }
    }
    // unary 
    if(n->ruleType == unaryExpression_rule){
        if(n->childCount == 1){
            return typeOfExpression(cstm, n->children[0]);
        // unary operator + unary expression
        }else if(n->children[1]->ruleType == unaryExpression_rule){
            int type1 = typeOfExpression(cstm, n->children[1]);
            if(type1 >= 0){
                fprintf(stderr, "%sError typeOfExpression line %d: unary operator cannot act on reference type object\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            return type1;
        // self operator + term
        }else{
            int type1 = typeOfTerm(cstm, n->children[1]);
            if(type1 >= 0){
                fprintf(stderr, "%sError typeOfExpression line %d: unary operator cannot act on reference type object\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            return type1;
        }
    }
    // postfix
    if(n->ruleType == postfixExpression_rule){
        if(n->childCount == 1){
            return typeOfTerm(cstm, n->children[0]);
        }else{
            int type0 = typeOfTerm(cstm, n->children[0]);
            if(type0 >= 0){
                fprintf(stderr, "%sError typeOfExpression line %d: unary operator cannot act on reference type object\n%s", RED, n->assoToken->lineNumber, NRM);
                exit(1);
            }
            return type0;
        } 
    }
    return -1;
}

bool isvalidInheritance(classSTManager* cstm, int super, int sub){
    if(!cstm || super<0 || sub<0){
        return false;
    }
    if(super == sub || !super){
        return true;
    }
    int tmp = sub;
    while(tmp){
        tmp = lookupIndexClassST(cstm, cstm->registeredTables[sub]->superclassGenerics->type);
        if(tmp == super){
            return true;
        }
    }
    return false;
}

bool checkStatement(classSTManager* cstm, treeNode* n){
    if(!cstm || !n){
        return false;
    }
    if(n->ruleType != statement_rule){
        return false;
    }
    if(n->childCount == 2){
        if(n->children[0]->ruleType == assignment_rule){
            if(!checkAssignmentCompatible(cstm, n->children[0])){
                return false;
            }
        }
    }
    // placeholder
    return true;
}


