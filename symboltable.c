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
    // post order traversal assures
    // we always first deal wtih child nodes
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            attachSymbolTables2Nodes(n->children[i]); 
        }
    }
    
    // class declaration
    if(n->ruleType == classDeclaration_rule || n->ruleType == interfaceDeclaration_rule){
        printf("Debug: attach class symbol table to class or interface declaration\n");
        n->classSymbolTable = attachClassSymbolTable(n);
    }
    
    // method declaration
    // classDec->classBody->subDec
    if(n->ruleType == subroutineDeclaration_rule){
        printf("Debug: attach method symbol table to method declaration\n");
        n->methodSymbolTable = attachMethodSymbolTable(n, n->parent->parent);
    }
    
    // argument list
    // subDec->paralist
    if(n->ruleType == parameterList_rule){
        printf("Debug: attach var symbol table to parameter list\n");
        n->varSymbolTable = attachArgListSymbolTable(n, n->parent);
        n->varCount += countCommas(n) + 1;
    }
    
    // variable declaration
    if(n->ruleType == variableDeclaration_rule){
        printf("Debug: attach var symbol table to variable declaration, \n");
        // class level field
        // classDec->classBody->varDec
        if(n->parent->ruleType==classBody_rule){
            printf("    parent = class.\n");
            n->varSymbolTable = attachVarSymbolTable(n, n->parent->parent, NULL, NULL);
            n->varCount += countCommas(n) + 1;
        }
        // method level local variable
        // subDec->subBody->varDec
        else if(n->parent->ruleType==subroutineBody_rule){
            printf("    parent = method.\n");
            n->varSymbolTable = attachVarSymbolTable(n, NULL, n->parent->parent, NULL);
            n->varCount += countCommas(n) + 1;
        }
        // compound level local variable
        // if, switch, for, while, do-while, static, code block
        // stmt->forStmt->stmt->varDec
        else if(n->parent->parent->ruleType==ifStatement_rule || n->parent->parent->ruleType==switchStatement_rule || n->parent->parent->ruleType==whileStatement_rule || n->parent->parent->ruleType==doWhileStatement_rule || n->parent->parent->ruleType==staticStatement_rule || n->parent->parent->ruleType==codeBlock_rule){
            printf("    parent = compound.\n");
            n->varSymbolTable = attachVarSymbolTable(n, NULL, NULL, n->parent->parent);
            n->varCount += countCommas(n) + 1;
        }
    }
    
}

classST* attachClassSymbolTable(treeNode* n){
    if(!n){
        fprintf(stderr, "%sError attachClassSymbolTable: tree node not passed in%s\n", RED, NRM);
        exit(1);
    }
    if(n->ruleType != classDeclaration_rule && n->ruleType != interfaceDeclaration_rule){
        fprintf(stderr, "%sError attachClassSymbolTable: non class declaration node%s\n", RED, NRM);
        exit(1);
    }
    classST* st = calloc(1, sizeof(classST));
    if(!st){
        fprintf(stderr, "%sError attachClassSymbolTable: not enough memory, cannot create a class symbol table%s\n", RED, NRM);
        exit(1);
    }
    st->attachNode = n;
    
    st->cf = CLASS_ST;
    if(n->ruleType == classDeclaration_rule){
        st->isClass = true;
    }
    if(n->ruleType == interfaceDeclaration_rule){
        st->isInterface = true;
    }
    
    treeNode* markClassBody = NULL;
    
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
        if(st->isClass && child->ruleType == extends_rule){
            if(n->children[i+2]->ruleType == generics_rule){
                st->superclassGenerics = attachGenericsSymbolTable(n->children[i+1]->assoToken->data.str_val, n->children[i+2]);
            }else{
                st->superclassGenerics = attachGenericsSymbolTable(n->children[i+1]->assoToken->data.str_val, NULL);
            }
        }
        
        // interfaces
        if((st->isClass && child->ruleType == implements_rule) || (st->isInterface && child->ruleType == extends_rule)){
            int commaCount = countCommas(n);
            st->interfacesCount = commaCount + 1;
            
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
                        
                        st->interfacesGenerics[idx] = attachGenericsSymbolTable(n->children[i+k]->assoToken->data.str_val, n->children[i+k+1]);
                    }else{
                        
                        st->interfacesGenerics[idx] = attachGenericsSymbolTable(n->children[i+k]->assoToken->data.str_val, NULL);
                    }
                    idx++;
                }
                k++;
            }
            
        }
        
        // mark position of classBody
        if(child->ruleType == classBody_rule){
            markClassBody = child;
        }
        
    }
    
    // if no superclass, then it extends Object
    if(st->isClass && !st->superclassGenerics){
        st->superclassGenerics = attachGenericsSymbolTable("Object", NULL);
    }
    
    // link child STs to current ST
    int fieldsCount = 0;
    int methodsCount = 0;
    for(size_t i=0; i<markClassBody->childCount; i++){
        if(markClassBody->children[i]->ruleType == variableDeclaration_rule){
            fieldsCount += markClassBody->children[i]->varCount;
        }
        if(markClassBody->children[i]->ruleType == subroutineDeclaration_rule){
            methodsCount++;
        }
    }
    //printf("Debug: fieldCount = %d, methodCount = %d\n", fieldCount, methodCount);
    st->fieldsCount = fieldsCount;
    st->fields = fieldsCount==0?NULL:calloc(fieldsCount, sizeof(varST*));

    st->methodsCount = methodsCount;
    st->methods = methodsCount==0?NULL:calloc(methodsCount, sizeof(methodST*));

    
    int field_idx = 0;
    int method_idx = 0;
    
    for(size_t i=0; i<markClassBody->childCount; i++){
        if(markClassBody->children[i]->ruleType == variableDeclaration_rule){
            for(size_t j=0; j<markClassBody->children[i]->varCount; j++){
                st->fields[field_idx] = markClassBody->children[i]->varSymbolTable[j];
                field_idx++;
            }
        }
        if(markClassBody->children[i]->ruleType == subroutineDeclaration_rule){
            st->methods[method_idx] = markClassBody->children[i]->methodSymbolTable;
            method_idx++;
        }
    }
    
    return st;
}

methodST* attachMethodSymbolTable(treeNode* n, treeNode* parentClass){
    if(!n){
        fprintf(stderr, "%sError attachMethodSymbolTable: tree node not passed in%s\n", RED, NRM);
        exit(1);
    }
    if(n->ruleType != subroutineDeclaration_rule){
        fprintf(stderr, "%sError attachMethodSymbolTable: not subroutine declaration node%s\n", RED, NRM);
        exit(1);
    }
    if(!parentClass){
        fprintf(stderr, "%sError attachMethodSymbolTable: parent class not passed in%s\n", RED, NRM);
        exit(1);
    }
    
    methodST* st = calloc(1, sizeof(methodST));
    if(!st){
        fprintf(stderr, "%sError attachMethodSymbolTable: not enough memory, cannot create a method symbol table%s\n", RED, NRM);
        exit(1);
    }
    st->cf = METHOD_ST;
    st->attachNode = n;
    st->parentClass = parentClass;
    
    treeNode* markBound = NULL;
    treeNode* markType = NULL;
    treeNode* markId = NULL;
    treeNode* markSubBody = NULL;
    
    for(size_t i=0; i < n->childCount; i++){
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
        
        // is abstract method?
        if(child->ruleType == abstract_rule){
            st->isAbstract = true;
        }
        
        // is native method?
        if(child->ruleType == native_rule){
            st->isNative = true;
        }
               
        // mark position of type boundedness
        if(child->ruleType == typeBoundList_rule){
            markBound = child;
        }
        // mark position of type
        if(child->ruleType == type_rule){
            markType = child;
        }
        // mark position of method name
        if(child->ruleType == identifier_rule){
            markId = child;
        }
        // mark position of subroutine body
        if(child->ruleType == subroutineBody_rule){
            markSubBody = child;
        }
        
        // attach argument list
        if(child->ruleType == parameterList_rule){
            st->argumentsCount = child->varCount;
            // this is a shallow copy, don't free this
            st->arguments = child->varSymbolTable;
        }
        
        // store annotation
        if(child->ruleType == annotation_rule){
            st->annotation = mystrdup(child->children[1]->assoToken->data.str_val);
        }
        
    }
    
    st->name = mystrdup(markId->assoToken->data.str_val);
    
    // if no return type, it's a constructor
    if(!markType){
        st->isConstructor = true;
    }
    
    // attach generics ST
    if(markBound){
        st->generics = attachTypeBoundSymbolTable(st->name, markBound);
    }
    
    // link child STs to current ST
    int localsCount = 0;
    for(size_t i=0; i<markSubBody->childCount; i++){
        if(markSubBody->children[i]->ruleType == variableDeclaration_rule){
            localsCount += markSubBody->children[i]->varCount;
        }
    } 
    st->localsCount = localsCount;
    st->locals = localsCount==0?NULL:calloc(localsCount, sizeof(methodST*));
    
    int local_idx = 0;
    for(size_t i=0; i<markSubBody->childCount; i++){
        if(markSubBody->children[i]->ruleType == variableDeclaration_rule){
            for(size_t j=0; j<markSubBody->children[i]->varCount; j++){
                st->locals[local_idx] = markSubBody->children[i]->varSymbolTable[j];
                local_idx++;
            }
        }
    }
    
    return st;
    
}

genST* attachTypeBoundSymbolTable(char* name, treeNode* typeBoundList) {
    assert(typeBoundList);
    assert(typeBoundList->ruleType == typeBoundList_rule);
    
    treeNode* tmp_gen = convertTypeBound2Generics(typeBoundList);
    
    genST* out = attachGenericsSymbolTable(name, tmp_gen);
    
    freeTreeNode(tmp_gen);
    
    return out;
}

treeNode* convertTypeBound2Generics(treeNode* typeBoundList){
    //treeNode* deepCopySubtree(treeNode* n);
    assert(typeBoundList);
    assert(typeBoundList->ruleType == typeBoundList_rule);
    
    // first count how many typeArgument we need
    int args = 0;
    for(size_t i=0; i < typeBoundList->childCount; i++){
        if(typeBoundList->children[i]->ruleType == typeBound_rule){
            if(typeBoundList->children[i]->childCount == 1){
                args++;
            }else{
                treeNode* constraint = typeBoundList->children[i]->children[1];
                for(size_t j=0; j < constraint->childCount; j++){
                    if(constraint->children[j]->ruleType == type_rule){
                        args++;
                    }
                }
            }
        }
    }
    //printf("%sargs = %d%s\n", RED, args, NRM);
    treeNode* root = createTreeNode(generics_rule, NULL);
    insertNewNode2Parent(langle_rule, NULL, root);
    for(size_t i=0; i < typeBoundList->childCount; i++){
        if(typeBoundList->children[i]->ruleType == typeBound_rule){
            // typeBound->identifier. the resulting generics is
            // typeAgr->refType->identifier
            if(typeBoundList->children[i]->childCount == 1){
                treeNode* typeArg = insertNewNode2Parent(typeArgument_rule, NULL, root);
                treeNode* ref = insertNewNode2Parent(referenceType_rule, NULL, typeArg);
                treeNode* id_copy = deepCopySubtree(typeBoundList->children[i]->children[0]);
                id_copy->parent = ref;
                insertChildNode(ref, id_copy);
                insertChildNode(root, typeArg);
                args--;
                if(args>0){
                    insertNewNode2Parent(comma_rule, NULL, root);
                }
            // typeBound->identifier(copy this)
            //          ->constraint->extends
            //                      ->type->refType(copy this)
            // resulting generics is 
            // typeArg->identifier(copy)
            //        ->extends
            //        ->refType(copy)
            }else{
                treeNode* id = typeBoundList->children[i]->children[0];
                treeNode* constraint = typeBoundList->children[i]->children[1];
                for(size_t j=0; j < constraint->childCount; j++){
                    if(constraint->children[j]->ruleType == type_rule){
                        treeNode* ref = constraint->children[j]->children[0];
                        treeNode* ref_copy = deepCopySubtree(ref);
                        
                        treeNode* typeArg = insertNewNode2Parent(typeArgument_rule, NULL, root);
                        // insert id_copy to typeArg
                        treeNode* id_copy = deepCopySubtree(id);
                        id_copy->parent = typeArg;
                        insertChildNode(typeArg, id_copy);
                        // insert extends to typeArg
                        insertNewNode2Parent(extends_rule, NULL, typeArg);
                        // insert refType_copy into typeArg
                        ref_copy->parent = typeArg;
                        insertChildNode(typeArg, ref_copy);
                        // insert typeArg into root
                        insertChildNode(root, typeArg);
                        // insert comma when eligible
                        args--;
                        if(args>0){
                            insertNewNode2Parent(comma_rule, NULL, root);
                        }
                    }
                }
            }
        }
    }
    insertNewNode2Parent(rangle_rule, NULL, root);
    
    printf("---------------------------------------\n");
    printLessTreeNode(root, 0);
    return root;
    
}

varST** attachVarSymbolTable(treeNode* n, treeNode* parentClass, treeNode* parentMethod, treeNode* parentCompound){
    if(!n){
        fprintf(stderr, "%sError attachVarSymbolTable: tree node not passed in%s\n", RED, NRM);
        exit(1);
    }
    if(n->ruleType != variableDeclaration_rule){
        fprintf(stderr, "%sError attachVarSymbolTable: non variable declaration node%s\n", RED, NRM);
        exit(1);
    }
    if(!parentClass && !parentMethod && !parentCompound){
        fprintf(stderr, "%sError attachVarSymbolTable: parent Symbol table not passed in%s\n", RED, NRM);
        exit(1);
    }
    
    // create the array of varST
    int commaCount = countCommas(n);
    varST** arr = calloc(commaCount + 1, sizeof(varST*));
    if(!arr){
        fprintf(stderr, "%sError attachVarSymbolTable: not enough memory, cannot create an array of variable symbol tables%s\n", RED, NRM);
        exit(1);
    }
    n->varSymbolTable = arr;
    
    // common properties of all vars
    bool isPublic = false;
    bool isPrivate = false;
    bool isStatic = false;
    bool isFinal = false;
    size_t arrDimension = 0; 
    
    treeNode* markType = NULL;
    int typeOption = 0;
    
    int idx_arr = 0;
    
    for(size_t i=0; i < n->childCount; i++){
        // the order for tokens is guaranteed by parser,
        // so here we can deal with them separately safely.
        treeNode* child = n->children[i];
        
        // accessibility
        if(child->ruleType == accessModifier_rule){
            if(child->assoToken->data.key_val == PUBLIC){
                isPublic = true;
            }
            else if(child->assoToken->data.key_val == PRIVATE){
                isPrivate = true;
            }
        }
        
        // is static or final?
        if(child->ruleType == nonAccessModifier_rule){
            if(child->assoToken->data.key_val == STATIC){
                isStatic = true;
            }
            else if(child->assoToken->data.key_val == FINAL){
                isFinal = true;
            }
        }
        
        if(child->ruleType == type_rule){
            markType = child;
            // primitive type
            if(child->children[0]->ruleType == primitiveType_rule){
                typeOption = 1;
                printf("  type = 1\n");
            }
            else if(child->children[0]->ruleType == referenceType_rule){
                // reference type without generics
                if(child->children[0]->childCount==1){
                    typeOption = 2;
                    printf("  type = 2\n");
                }
                // reference type with generics
                else{
                    typeOption = 3;
                    printf("  type = 3\n");
                }
            }
        }
        
        // if arrDimention>0, meaning we have already dealt with brackets
        if(child->ruleType == bracket_rule){
            int bracketCount = countBrackets('[', ']', n);
            assert(bracketCount%2==0);
            arrDimension = bracketCount / 2;
        }
        
        // identifier or assignment
        if(child->ruleType == identifier_rule || child->ruleType == assignment_rule){
            varST* st = calloc(1, sizeof(varST));
            if(!st){
                fprintf(stderr, "%sError attachVarSymbolTable: not enough memory, cannot create a variable symbol table%s\n", RED, NRM);
                exit(1);
            }
            st->attachNode = n;
            st->cf = VAR_ST;
            if(parentClass){
                st->parentClass = parentClass;
            }else if(parentMethod){
                st->parentMethod = parentMethod;
            }else if(parentCompound){
                st->parentCompound = parentCompound;
            }
            if(child->ruleType == identifier_rule){
                st->name = mystrdup(child->assoToken->data.str_val);
            }
            // assignment -> term ->identifier
            else if(child->ruleType == assignment_rule){
                st->name = mystrdup(child->children[0]->children[0]->assoToken->data.str_val);
            }
            
            switch(typeOption){
                case 1:
                    st->type = attachGenericsSymbolTable(getKeyword(markType->children[0]->assoToken->data.key_val), NULL);
                    break;
                case 2:
                    st->type = attachGenericsSymbolTable(markType->children[0]->children[0]->assoToken->data.str_val, NULL);
                    break;
                case 3:
                    st->type = attachGenericsSymbolTable(markType->children[0]->children[0]->assoToken->data.str_val, markType->children[0]->children[1]);
                    break;
                default:
                    fprintf(stderr, "%sinvalid type attaching var ST%s\n", RED, NRM);
                    exit(1);
            }
            
            st->isPublic = isPublic;
            st->isPrivate = isPrivate;
            st->isStatic = isStatic;
            st->isFinal = isFinal;
            st->arrDimension = arrDimension;
            
            arr[idx_arr] = st;
            idx_arr++;
        }  
    }
    
    return arr;
}

varST** attachArgListSymbolTable(treeNode* n, treeNode* parentMethod){
    if(!n){
        fprintf(stderr, "%sError attachArgListSymbolTable: tree node not passed in%s\n", RED, NRM);
        exit(1);
    }
    if(n->ruleType != parameterList_rule){
        fprintf(stderr, "%sError attachArgListSymbolTable: not parameter declaration node%s\n", RED, NRM);
        exit(1);
    }
    if(!parentMethod){
        fprintf(stderr, "%sError attachArgListSymbolTable: parent method not passed in%s\n", RED, NRM);
        exit(1);
    }
    
    // create the array of varST
    int commaCount = countCommas(n);
    varST** arr = calloc(commaCount + 1, sizeof(varST*));
    if(!arr){
        fprintf(stderr, "%sError attachArgListSymbolTable: not enough memory, cannot create an array of parameter symbol tables%s\n", RED, NRM);
        exit(1);
    }
    n->varSymbolTable = arr;
    
    int idx_arr = 0;
    
    for(size_t i=0; i < n->childCount; i++){
        treeNode* prevchild = (int)i-1>=0?n->children[i-1]:NULL;
        treeNode* child = n->children[i];
        treeNode* nextchild = i+1<n->childCount?n->children[i+1]:NULL;
        
        // create new var ST for every <type>
        if(child->ruleType == type_rule){
            varST* st = calloc(1, sizeof(varST));
            if(!st){
                fprintf(stderr, "%sError attachVarSymbolTable: not enough memory, cannot create a variable symbol table%s\n", RED, NRM);
                exit(1);
            }
            st->attachNode = n;
            st->cf = VAR_ST;
            st->parentMethod = parentMethod;
            
            // is const?
            if(prevchild && prevchild->ruleType == const_rule){
                printf("Debug: const argument\n");
                st->isFinal = true;
            }
            
            // if nextchile is bracket,
            // first count array dimension,
            // then find the next identifier
            if(nextchild->ruleType == bracket_rule){
                int bracketCount = 0;
                int k = 1;
                while(n->children[i+k]->ruleType == bracket_rule){
                    bracketCount++;
                    k++;
                }
                assert(bracketCount%2==0);
                st->arrDimension = bracketCount / 2;
                if(n->children[i+k]->ruleType != identifier_rule){
                    fprintf(stderr, "%slogical error finding identifier after array%s\n", RED, NRM);
                    exit(1);
                }
                st->name = mystrdup(n->children[i+k]->assoToken->data.str_val);
            }else{
                st->name = mystrdup(nextchild->assoToken->data.str_val);
            }
            
            // finally deal with the type
            // primitive type
            if(child->children[0]->ruleType == primitiveType_rule){
                st->type = attachGenericsSymbolTable(getKeyword(child->children[0]->assoToken->data.key_val), NULL);
            }
            else if(child->children[0]->ruleType == referenceType_rule){
                // reference type without generics
                if(child->children[0]->childCount==1){
                    st->type = attachGenericsSymbolTable(child->children[0]->children[0]->assoToken->data.str_val, NULL);
                }
                // reference type with generics
                else{
                    st->type = attachGenericsSymbolTable(child->children[0]->children[0]->assoToken->data.str_val, child->children[0]->children[1]);
                }
            }
            
            arr[idx_arr] = st;
            idx_arr++;
        }
        
    }

    return arr;
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
    
    // case 2. gen=NULL
    if(!gen && type){
        st->type = mystrdup(type);
        return st;
    }
    
    // case 1, gen should be of typeArgument type to process only one generic
    if(!type && gen){
        
        if(gen->ruleType!=typeArgument_rule){
            fprintf(stderr, "%sError attachGenericsSymbolTable: in nested generics the gen node passed to function should be of typeArgument rule type%s\n", RED, NRM);
            exit(1);
        }
        
        // a standalone typeArgument->referencetype->identifier
        if(gen->children[0]->ruleType==referenceType_rule){
            st->name = mystrdup(gen->children[0]->children[0]->assoToken->data.str_val);
        }
        // the first token is either wildcard or an identifier
        else if(gen->children[0]->ruleType==identifier_rule){
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
            int commaCount = countCommas(ref->children[1]);
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
    int commaCount = countCommas(gen);
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

int countCommas(treeNode* n){
    int commaCount = 0;
    for(size_t i=0; i<n->childCount; i++){
        if(n->children[i]->ruleType==comma_rule){
            commaCount++;
        }
    }
    return commaCount;
}

int countBrackets(char c1, char c2, treeNode* n){
    int count = 0;
    for(size_t i=0; i<n->childCount; i++){
        if(n->children[i]->ruleType==bracket_rule){
            if(n->children[i]->assoToken->data.char_val==c1 || n->children[i]->assoToken->data.char_val==c2){
                count++;
            }
        }
    }
    return count;
}

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
    if(n->ruleType == subroutineDeclaration_rule){
        printMethodST(n->methodSymbolTable);
        flag = true;
    }
    if(n->ruleType == variableDeclaration_rule){
        for(size_t i=0; i < n->varCount; i++){
            printVarST(n->varSymbolTable[i]);
        }
        flag = true;
    }
    if(n->ruleType == forStatement_rule){
       
        for(size_t i=0; i<n->varCount; i++){
            printVarST(n->varSymbolTable[i]);
        }
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
    printf("Class =");
    printGenericsST(st->generics);
    printf(", extends");
    printGenericsST(st->superclassGenerics);
    if(st->interfacesGenerics){
        printf(", implements");
        for(size_t i=0; i<st->interfacesCount; i++){
            printGenericsST(st->interfacesGenerics[i]);
        }
    }
    printf(", parent = %s", getRule(st->attachNode->ruleType));
    printf(".\n");
}

void printMethodST(methodST* st){
    if(!st){
        return;
    }
    if(st->isPublic){
        printf("public ");
    }else if(st->isPrivate){
        printf("private ");
    }
    
    if(st->isStatic){
        printf("static ");
    }else if(st->isFinal){
        printf("final ");
    }
    
    if(st->isAbstract){
        printf("abstract ");
    }
    
    printf("methodName = %s", st->name);
    
    printf(", returnType = ");
    if(!st->returnType){
        printf("void ");
    }else{
        printGenericsST(st->returnType);
    }
    
    printf(", type boundedness = ");
    printGenericsST(st->generics);
    
    if(st->arguments){
        printf(", arguments: ");
        for(size_t i=0; i<st->argumentsCount; i++){
            printVarST(st->arguments[i]);
            printf(", ");
        }
    }
    printf("\n");
    
}

void printVarST(varST* st){
    if(!st){
        printf("empty\n");
        return;
    }
    
    if(st->isPublic){
        printf("public ");
    }else if(st->isPrivate){
        printf("private ");
    }
    
    if(st->isStatic){
        printf("static ");
    }else if(st->isFinal){
        printf("final ");
    }
    
    printf("varName = %s,", st->name);
    printf(" varType =");
    printGenericsST(st->type);
    
    if(st->arrDimension>0){
        printf(", array dimension = %ld", st->arrDimension);
    }
    printf("\n");
}

void printGenericsST(genST* st){
    if(!st){
        return;
    }
    
    if(st->name){
        printf("%s", st->name);
    }else if(st->isWildcard){
        printf("?");
    }
    
    if(st->extends){
        printf(" extends");
    }else if(st->super){
        printf(" super");
    }
    if(st->type){
        printf(" %s", st->type);
    }
    
    if(st->nestedCount>0){
        printf("<");
        for(size_t i=0; i<st->nestedCount; i++){
            printGenericsST(st->nested[i]);
            if(st->nestedCount>1 && i!=st->nestedCount-1){
                printf(", ");
            }
        }
        printf("> ");
    }
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
            free(n->varSymbolTable[i]);
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
    free(st->fields);
    free(st->methods);
}

void freeMethodST(methodST* st){
    if(!st){
        return;
    }
    free(st->name);
    free(st->annotation);
    freeGenericsST(st->returnType);
    free(st->returnType);
    freeGenericsST(st->generics);
    free(st->generics);
    free(st->locals);
}

void freeVarST(varST* st){
    if(!st){
        return;
    }
    freeGenericsST(st->type);
    free(st->type);
    free(st->name);
}

void freeGenericsST(genST* st){
    if(!st){
        return;
    }
    free(st->name);
    free(st->type);
    if(st->nestedCount){
        for(size_t i=0; i<st->nestedCount; i++){
            freeGenericsST(st->nested[i]);
            free(st->nested[i]);
        }
    }
    free(st->nested);
}

void* testcalloc(size_t len, size_t size){
    return calloc(len, size);
}
