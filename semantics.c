#include "semantics.h"

bool checkCSTM(classSTManager* cstm){
    if(!cstm){
        return false;
    }
    if(!cstm->hasMain){
        fprintf(stderr, "%sWarning checkClassInheritance: program does not have main function%s", RED, NRM);
    }
    bool record = true;
    for(size_t i=0; i < cstm->length; i++){
        classST* st = cstm->registeredTables[i];
        if(st->isClass){
            record = checkClassInheritance(cstm, st) & record;
            record = checkInterfaceImplementation(cstm, st) & record;
        }
    }
    return record;
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
                printf("Debug: check method %s for %s\n", st->methods[k]->generics->type, super->virtualTable->entries[j]->generics->type);
                if(methodOverrides(st->methods[k], super->virtualTable->entries[j])){  
                printf("possible?\n");
                if(isValidOverrideReturnType(st->methods[k], super->virtualTable->entries[j], cstm)
                ){
                    found = true;
                }}
            }
            if(!found){
                fprintf(stderr, "%sError checkInterfaceImplementation: valid implementation in %s of abstract method %s from interface %s not found%s\n", RED, st->generics->type, super->virtualTable->entries[j]->generics->type, st->interfacesGenerics[i]->type, NRM);
                //exit(1);
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

