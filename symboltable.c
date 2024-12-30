#include "symboltable.h"

void attachSymbolTables(CST* cst){
    if(!cst || !cst->root){
        return;
    }
    attachSymbolTables2Nodes(cst->root);
}

void attachSymbolTables2Nodes(treeNode* n){
    if(!n){
        return;
    }
    if(n->ruleType == classDeclaration_rule || n->ruleType == interfaceDeclaration_rule){
        printf("Debug: attach symbol table to class or interface declaration\n");
        attachClassSymbolTable(n);
    }
    // placeholder for method and var ST
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            attachSymbolTables2Nodes(n->children[i]); 
        }
    }
}

void attachClassSymbolTable(treeNode* n){
    if(n->ruleType != classDeclaration_rule && n->ruleType != interfaceDeclaration_rule){
        fprintf(stderr, "%sError attachClassSymbolTable: non class declaration node%s\n", RED, NRM);
        exit(1);
    }
    classST* st = calloc(1, sizeof(classST));
    if(!st){
        fprintf(stderr, "%sError attachClassSymbolTable: not enough memory, cannot create a class symbol table%s\n", RED, NRM);
        exit(1);
    }
    st->parentNode = n;
    
    st->cf = CLASS_ST;
    if(n->ruleType == classDeclaration_rule){
        st->isClass = true;
    }
    if(n->ruleType == interfaceDeclaration_rule){
        st->isInterface = true;
    }
    
    for(size_t i=0; i < n->childCount; i++){
        // the order for tokens is guaranteed by parser,
        // so here we can deal with them separately safely.
        treeNode* child = n->children[i];
        
        // accessibility
        if(child->ruleType == accessModifier_rule){
            if(child->assoToken->data.key_val == PUBLIC){
                st->isPublic = true;
            }
            else if(child->assoToken->data.key_val == PRIVATE){
                st->isPrivate = true;
            }
        }
        
        // is static or final?
        if(child->ruleType == nonAccessModifier_rule){
            if(child->assoToken->data.key_val == STATIC){
                st->isStatic = true;
            }
            else if(child->assoToken->data.key_val == FINAL){
                st->isFinal = true;
            }
        }
        
        // is abstract class?
        if(child->ruleType == abstract_rule){
            st->isAbstract = true;
        }
        
        // class name with generics
        if(child->ruleType == class_rule || child->ruleType == interface_rule){
            if(n->children[i+2]->ruleType == generics_rule){
                st->generics = attachGenericsSymbolTable(n->children[i+1]->assoToken->data.str_val, n->children[i+2]);
            }else{
                st->generics = attachGenericsSymbolTable(n->children[i+1]->assoToken->data.str_val, NULL);
            }
        }
        
        // superclass
        if(child->ruleType == extends_rule){
            if(n->children[i+2]->ruleType == generics_rule){
                st->superclassGenerics = attachGenericsSymbolTable(n->children[i+1]->assoToken->data.str_val, n->children[i+2]);
            }else{
                st->superclassGenerics = attachGenericsSymbolTable(n->children[i+1]->assoToken->data.str_val, NULL);
            }
        }
        
        // interfaces
        if(child->ruleType == implements_rule){
            int commaCount = 0;
            for(size_t j=0; j < n->childCount; j++){
                if(n->children[j]->ruleType == comma_rule){
                    commaCount++;
                }
            }
            st->interfacesCount = commaCount + 1;
            printf("Debug: implemented %ld interfaces\n", st->interfacesCount);
            st->interfacesGenerics = calloc(st->interfacesCount, sizeof(genST*));
            if(!st->interfacesGenerics){
                fprintf(stderr, "%sError attachClassSymbolTable: not enough memory, cannot create a array of interfaces%s\n", RED, NRM);
                exit(1);
            }
            
            size_t k = 1;
            size_t idx = 0;
            while(n->children[i+k]->ruleType != compound_rule){
                if(n->children[i+k]->ruleType == identifier_rule){
                    if(n->children[i+k+1]->ruleType == generics_rule){
                        printf("Debug: insert interface with generics\n");
                        st->interfacesGenerics[idx] = attachGenericsSymbolTable(n->children[i+k]->assoToken->data.str_val, n->children[i+k+1]);
                    }else{
                        printf("Debug: insert interface without generics\n");
                        st->interfacesGenerics[idx] = attachGenericsSymbolTable(n->children[i+k]->assoToken->data.str_val, NULL);
                    }
                    idx++;
                }
                k++;
            }
            
        }
        
    }
    
    // if no superclass, then it extends Object
    if(!st->superclassGenerics){
        st->superclassGenerics = attachGenericsSymbolTable("Object", NULL);
    }
    
    n->classSymbolTable = st;
}

genST* attachGenericsSymbolTable(char* type, treeNode* gen){
    if(!type && !gen){
        return NULL;
    }
    genST* st = calloc(1, sizeof(genST));
    if(!st){
        fprintf(stderr, "%sError attachGenericsSymbolTable: not enough memory, cannot create a generics symbol table%s\n", RED, NRM);
        exit(1);
    }
    st->cf = GEN_ST;
    
    // 3 branches: 
    // 1. type=NULL. E extends Number
    // 2. gen=NULL. input is a type without generics. String
    // 3. type!=NULL, gen!=NULL. the outermost part of a generics. List<...>
    if(!gen && type){
        st->type = mystrdup(type);
        return st;
    }
    
    // gen should be of typeArgument type to process only one generic
    if(!type && gen){
        if(gen->ruleType!=typeArgument_rule){
            fprintf(stderr, "%sError attachGenericsSymbolTable: in nested generics the gen node passed to function should be of typeArgument rule type%s\n", RED, NRM);
            exit(1);
        }
        // the first token is either wildcard or an identifier
        if(gen->children[0]->ruleType==identifier_rule){
            st->name = mystrdup(gen->children[0]->assoToken->data.str_val);
        }else if(gen->children[0]->ruleType==wildcard_rule){
            st->isWildcard = true;
        }
        
        if(gen->childCount==1){
            return st;
        }
        // childCount > 1, next node must be either extends or super
        if(gen->children[1]->ruleType==extends_rule){
            st->extends = true;
        }else if(gen->children[1]->ruleType==super_rule){
            st->super = true;
        }
        // extends or super a reference type, which is futher decomposed 
        // into identifier + optional generics
        treeNode* ref = gen->children[2];
        if(ref->ruleType!=referenceType_rule){
            fprintf(stderr, "%sthe parser has gone wrong. this node must be a reference type.%s\n", RED, NRM);
            exit(1);
        }
        // referenceType->identifier is the type of current ST
        st->type = mystrdup(ref->children[0]->assoToken->data.str_val);
        // if referenceType has no further generics, end here
        if(ref->childCount==1){
            return st;
        }
        
        // if childCount of ref->generics > 1, count number of typeArguments first
        if(ref->children[1]->childCount > 1){
            int commaCount = 0;
            for(size_t i=0; i<ref->children[1]->childCount; i++){
                if(ref->children[1]->children[i]->ruleType==comma_rule){
                    commaCount++;
                }
            }
            st->nestedCount = commaCount + 1;
            st->nested = calloc(st->nestedCount, sizeof(genST*));
            // then recursively deal with them
            size_t idx_nested = 0;
            for(size_t i=0; i<ref->children[1]->childCount; i++){
                if(ref->children[1]->children[i]->ruleType==typeArgument_rule){
                    st->nested[idx_nested] = attachGenericsSymbolTable(NULL, ref->children[1]->children[i]);
                    idx_nested++;
                }
            }
        }
        return st;
    }
    
    // case 3. type!=NULL, gen!=NULL.
    st->type = mystrdup(type);
    // almost same code as above, but now gen must be of generics type
    if(gen->ruleType!=generics_rule){
        fprintf(stderr, "%sError attachGenericsSymbolTable: at first call of generics the gen node passed to function should be of generics rule type%s\n", RED, NRM);
        exit(1);
    }
    int commaCount = 0;
    for(size_t i=0; i<gen->childCount; i++){
        if(gen->children[i]->ruleType==comma_rule){
            commaCount++;
        }
    }
    st->nestedCount = commaCount + 1;
    st->nested = calloc(st->nestedCount, sizeof(genST*));
    size_t idx_nested = 0;
    for(size_t i=0; i<gen->childCount; i++){
        if(gen->children[i]->ruleType==typeArgument_rule){
            st->nested[idx_nested] = attachGenericsSymbolTable(NULL, gen->children[i]);
            idx_nested++;
        }
    }
    
    return st;
}

void attachMethodSymbolTable(treeNode* n, classST* parentClass){}
void attachVarSymbolTable(treeNode* n, classST* parentClass, methodST* methodST, varST* parentCompound){}


void printSymbolTables(CST* cst){
    if(!cst || !cst->root){
        return;
    }
    printNodeSymbolTable(cst->root, 0);
}

void printNodeSymbolTable(treeNode* n, int indent){
    if(!n){
        return;
    }
    bool flag = false;
    if(n->ruleType == classDeclaration_rule || n->ruleType == interfaceDeclaration_rule){
        printClassST(n->classSymbolTable);
        flag = true;
    }
    
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            printNodeSymbolTable(n->children[i], indent+(flag?2:0)); 
        }
    }
}

void printClassST(classST* st){
    if(!st){
        return;
    }
    printf("Class = ");
    printGenericsST(st->generics);
    printf(", extends ");
    printGenericsST(st->superclassGenerics);
    if(st->interfacesGenerics){
        printf(", implements ");
        for(size_t i=0; i<st->interfacesCount; i++){
            printGenericsST(st->interfacesGenerics[i]);
        }
    }
    printf(", parent = %s", getRule(st->parentNode->ruleType));
    printf(".\n");
}

void printMethodST(methodST* st){
    if(!st){
        return;
    }
    // placeholder
}

void printVarST(varST* st){
    if(!st){
        return;
    }
    // placeholder
}

void printGenericsST(genST* st){
    if(!st){
        return;
    }
    
    if(st->name){
        printf("%s ", st->name);
    }else if(st->isWildcard){
        printf("? ");
    }
    
    if(st->extends){
        printf("extends ");
    }else if(st->super){
        printf("super ");
    }
    
    printf("%s ", st->type);
    printf("<");
    
    if(st->nestedCount>0){
        for(size_t i=0; i<st->nestedCount; i++){
            printGenericsST(st->nested[i]);
        }
    }
    
    printf("> ");
}

void freeSymbolTables(CST* cst){
    if(!cst || !cst->root){
        return;
    }
    freeNodeSymbolTables(cst->root);
}

void freeNodeSymbolTables(treeNode* n){
    if(!n){
        return;
    }
    freeClassST(n->classSymbolTable);
    free(n->classSymbolTable);
    freeMethodST(n->methodSymbolTable);
    free(n->methodSymbolTable);
    if(n->varSymbolTable){
        for(size_t i=0; i<n->varCount; i++){
            freeVarST(n->varSymbolTable[i]); 
        }
    }
    free(n->varSymbolTable);
    
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            freeNodeSymbolTables(n->children[i]); 
        }
    }
}

void freeClassST(classST* st){
    if(!st){
        return;
    }
    freeGenericsST(st->generics);
    free(st->generics);
    freeGenericsST(st->superclassGenerics);
    free(st->superclassGenerics);
    if(st->interfacesGenerics){
        for(size_t i=0; i<st->interfacesCount; i++){
            freeGenericsST(st->interfacesGenerics[i]);
            free(st->interfacesGenerics[i]);
        }
    }
    free(st->interfacesGenerics);
    if(st->fields){
        for(size_t i=0; i<st->fieldsCount; i++){
            freeVarST(st->fields[i]);
            free(st->fields[i]);
        }
    }
    free(st->fields);
    if(st->methods){
        for(size_t i=0; i<st->methodsCount; i++){
            freeMethodST(st->methods[i]);
            free(st->methods[i]);
        }
    }
    free(st->methods);
}

void freeMethodST(methodST* st){
    if(!st){
        return;
    }
    freeGenericsST(st->generics);
    free(st->generics);
    
}

void freeVarST(varST* st){
    if(!st){
        return;
    }
    freeGenericsST(st->type);
    free(st->type);
    free(st->name);
    free(st->arrSizes);
}

void freeGenericsST(genST* st){
    if(!st){
        return;
    }
    free(st->name);
    free(st->type);
    if(st->nested){
        for(size_t i=0; i<st->nestedCount; i++){
            freeGenericsST(st->nested[i]);
            free(st->nested[i]);
        }
    }
    free(st->nested);
}

