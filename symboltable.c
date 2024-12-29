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
    if(!type){
        fprintf(stderr, "%sError attachGenericsSymbolTable: missing name for generics%s\n", RED, NRM);
        exit(1);
    }
    genST* st = calloc(1, sizeof(genST));
    if(!st){
        fprintf(stderr, "%sError attachGenericsSymbolTable: not enough memory, cannot create a generics symbol table%s\n", RED, NRM);
        exit(1);
    }
    
    st->type = calloc((int)strlen(type)+1, sizeof(char));
    if(!st->type){
        fprintf(stderr, "%sError attachGenericsSymbolTable: not enough memory, cannot create the name for a generics%s\n", RED, NRM);
        exit(1);
    }
    strcpy(st->type, type);
    
    // placeholder
    
    return st;
}


/*
varST** summarizeVarDec(treeNode* n, int* varCount){
    if(n->ruleType != variableDeclaration_rule){
        fprintf(stderr, "%sError summarizeVarDec: non variable declaration node%s\n", RED, NRM);
        exit(1);
    }
    if(!varCount){
        fprintf(stderr, "%sError summarizeVarDec: no variable counter%s\n", RED, NRM);
        exit(1);
    }
    *varCount = 1;
    varST* st = calloc(1, sizeof(varST));
    if(!st){
        fprintf(stderr, "%sError summarizeVarDec: not enough memory, cannot create variable symbol table%s\n", RED, NRM);
        exit(1);
    }
    st->cf = VAR_ST;
     
}
*/

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
    printf("ClassName = %s, extends %s, ", st->generics->type, st->superclassGenerics->type);
    if(st->interfacesGenerics){
        printf("implements ");
        for(size_t i=0; i<st->interfacesCount; i++){
            printf("%s, ", st->interfacesGenerics[i]->type);
        }
    }
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
    // placeholder
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
    // placeholder
}

void freeVarST(varST* st){
    if(!st){
        return;
    }
    //placeholder
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

