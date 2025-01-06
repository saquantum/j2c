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
    // so higher level ST can easily extract metadata
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            attachSymbolTables2Nodes(n->children[i]); 
        }
    }
    
    // class declaration
    if(n->ruleType == classDeclaration_rule || n->ruleType == interfaceDeclaration_rule){
        printf("Debug: attach class symbol table to class or interface declaration\n");
        n->classSymbolTable = attachClassSymbolTable(n);
        assignUniqueName(n->classSymbolTable);
    }
    
    // method declaration
    // classDec->classBody->subDec
    if(n->ruleType == subroutineDeclaration_rule || n->ruleType == subroutinePrototype_rule){
        printf("Debug: attach method symbol table to method declaration\n");
        n->methodSymbolTable = attachMethodSymbolTable(n, n->parent->parent);
        methodAccess(n->methodSymbolTable);
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
        if(child->ruleType == classBody_rule || child->ruleType == interfaceBody_rule){
            markClassBody = child;
        }
        
    }
    
    // if no superclass, then it extends Object
    if(st->isClass && !st->superclassGenerics){
        st->superclassGenerics = attachGenericsSymbolTable("Object", NULL);
    }
    
    if(st->isClass){
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
    }
    
    if(st->isInterface){
        int methodsCount = 0;
        for(size_t i=0; i<markClassBody->childCount; i++){
            if(markClassBody->children[i]->ruleType == subroutinePrototype_rule){
                methodsCount++;
            }
        }
        st->methodsCount = methodsCount;
        st->methods = methodsCount==0?NULL:calloc(methodsCount, sizeof(methodST*));
        int method_idx = 0;
        for(size_t i=0; i<markClassBody->childCount; i++){
            if(markClassBody->children[i]->ruleType == subroutinePrototype_rule){
                st->methods[method_idx] = markClassBody->children[i]->methodSymbolTable;
                method_idx++;
            }
        }
    }

    return st;
}

methodST* attachMethodSymbolTable(treeNode* n, treeNode* parentClass){
    if(!n){
        fprintf(stderr, "%sError attachMethodSymbolTable: tree node not passed in%s\n", RED, NRM);
        exit(1);
    }
    if(n->ruleType != subroutineDeclaration_rule && n->ruleType != subroutinePrototype_rule){
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
    
    int arrCount = 0;
    
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
            // if next child is bracket, then count array dimension
            int k = 1;
            while(n->children[i+k]->ruleType == bracket_rule){
                arrCount++;
                k++;
            }
            st->arrDimension = arrCount/2;
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
    
    // if no return type, it's a constructor
    if(!markType){
        st->isConstructor = true;
    }else{
        if(markType->childCount == 0){
            st->returnType = attachGenericsSymbolTable(getKeyword(markType->assoToken->data.key_val), NULL);
        }else if(markType->children[0]->ruleType == primitiveType_rule){
            st->returnType = attachGenericsSymbolTable(getKeyword(markType->children[0]->assoToken->data.key_val), NULL);
            st->returnsPrimitive = true;
        }else{
            if(markType->children[0]->childCount>0){
                st->returnType = attachGenericsSymbolTable(markType->children[0]->children[0]->assoToken->data.str_val, markType->children[0]->children[1]);
            }else{
                st->returnType = attachGenericsSymbolTable(markType->children[0]->children[0]->assoToken->data.str_val, NULL);
            }
        }
        
    }
    
    // attach generics ST
    st->generics = attachTypeBoundSymbolTable(markId->assoToken->data.str_val, markBound);
    
    
    if(markSubBody){
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
    }
    return st;
    
}

genST* attachTypeBoundSymbolTable(char* name, treeNode* typeBoundList) {

    if(typeBoundList){
        assert(typeBoundList->ruleType == typeBoundList_rule);
    
        treeNode* tmp_gen = convertTypeBound2Generics(typeBoundList);
    
        genST* out = attachGenericsSymbolTable(name, tmp_gen);
    
        freeTreeNode(tmp_gen);
    
        return out;
    }
    
    return attachGenericsSymbolTable(name, NULL);
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
            treeNode* id = typeBoundList->children[i]->children[0];
            // typeBound->identifier. the resulting generics is
            // typeAgr->refType->identifier
            if(typeBoundList->children[i]->childCount == 1){
                treeNode* typeArg = insertNewNode2Parent(typeArgument_rule, NULL, root);
                treeNode* ref = insertNewNode2Parent(referenceType_rule, NULL, typeArg);
                treeNode* id_copy = deepCopySubtree(id);
                id_copy->parent = ref;
                insertChildNode(ref, id_copy);
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
    
    //printf("---------------------------------------\n");
    //printLessTreeNode(root, 0);
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
            
            // if nextchild is bracket,
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

classSTManager* createCSTM(){
    classSTManager* cstm = calloc(1, sizeof(classSTManager));
    assert(cstm);
    cstm->registeredTables = calloc(16, sizeof(classST*));
    assert(cstm->registeredTables);
    cstm->capacity = 16;
    return cstm;
}

void insert2CSTM(classSTManager* cstm, classST* st){
    if(!cstm || !st){
        return;
    }
    if(cstm->capacity == cstm->length){
        cstm->registeredTables = (classST**)realloc(cstm->registeredTables, 2*(cstm->capacity)*sizeof(classST*));
        assert(cstm->registeredTables);
        cstm->capacity = 2 * cstm->capacity;
    }
    cstm->registeredTables[cstm->length++] = st;
}

void insertClass2CSTM(classSTManager* cstm, CST* cst){
    if(!cstm || !cst || !cst->root){
        return;
    }
    
    insertClass2CSTMHelper(cstm, cst->root);
}

void insertClass2CSTMHelper(classSTManager* cstm, treeNode* n){
    if(!n){
        return;
    }
    if(n->classSymbolTable){
        insert2CSTM(cstm, n->classSymbolTable);
        return; // this return skips all child nodes, screening all inner classes
    }
    if(n->childCount>0){
        for(size_t i=0; i < n->childCount; i++){
            insertClass2CSTMHelper(cstm, n->children[i]);
        }
    }
}

classST* lookupClassST(classSTManager* cstm, char* className){
    if(!cstm || !className){
        return NULL;
    }
    for(size_t i=0; i < cstm->length; i++){
        if(!strcmp(cstm->registeredTables[i]->generics->type, className)){
            return cstm->registeredTables[i];
        }
    }
    return NULL;
}

bool isVirtualMethod(methodST* st){
    if(!st || st->isPrivate || st->isStatic || st->isFinal || st->isConstructor){
        return false;
    }
    return true;
}

void assignUniqueName(classST* st){
    if(!st || !st->methods){
        return;
    }
    char tmp[256] = {0};
    for(size_t i=0; i < st->methodsCount; i++){
        strcpy(tmp, "");
        int count = 0;
        for(size_t j=0; j<i; j++){
            if(!strcmp(st->methods[j]->generics->type, st->methods[i]->generics->type)){
                count++;
            }
        }
        strcat(tmp, st->generics->type);
        strcat(tmp, "$");
        strcat(tmp, st->methods[i]->generics->type);
        strcat(tmp, "$");
        sprintf(tmp+strlen(tmp), "%d", count);
        st->methods[i]->name = mystrdup(tmp);
    }
    
}

int methodAccess(methodST* st){
    if(!st){
        return 0;
    }
    
    if(st->isPublic){
        return 4;
    }
    if(!st->isPrivate){
        return 2;
    }
    return 1;
}

bool areGenericsEqual(genST* st1, genST* st2, int mode){
    if(!st1 && !st2){
        return true;
    }
    if(!st1 || !st2){
        return false;
    }
    
    if( (st1->name && !st2->name) || (st2->name && !st1->name) ){
        return false;
    }
    
    // if mode<=0, we ignore name
    // <K extends Number> and <T extends Number>
    // mode=0 -> equal
    // mode=1 -> not equal
    if(mode>0 && st1->name && st2->name && strcmp(st1->name, st2->name)){
        return false;
    }
    
    if(st1->extends != st2->extends){
        return false;
    }
    if(st1->super != st2->super){
        return false;
    }
    
    if(!st1->type || !st2->type){
        return false;
    }
    if(strcmp(st1->type, st2->type)){
        return false;
    }
    
    if(st1->nestedCount != st2->nestedCount){
        return false;
    }
    
    if(st1->nestedCount){
        for(size_t i=0; i < st1->nestedCount; i++){
            if(!areGenericsEqual(st1->nested[i], st2->nested[i], mode)){
                return false;
            }
        }
    }
    
    return true;
}

bool methodOverridesSimpleChecks(methodST* st1, methodST* st2){
    if(!st1 || !st2){
        return false;
    }
    // they have the same name?
    if(strcmp(st1->generics->type, st2->generics->type)){
        return false;
    }
    // they have equal number of arguments?
    if(st1->argumentsCount != st2->argumentsCount){
        return false;
    }
    if(st1->generics->nestedCount != st2->generics->nestedCount){
        return false;
    }
    // if they dont have bounds, then we can directly compare type of arguments
    if(st1->generics->nestedCount==0){
        for(size_t i=0; i < st1->argumentsCount; i++){
            // argument types are identifier-sensitive, mode=1
            if(!areGenericsEqual(st1->arguments[i]->type, st2->arguments[i]->type, 1)){
                return false;
            }
        }
        return true;
    }
    // simple checks passed
    return true;
}

bool methodOverrides(methodST* st1, methodST* st2){
    if(!methodOverridesSimpleChecks(st1, st2)){
        return false;
    }
    
    // <T extends Number> void method(T t)
    // <K extends Number> void method(K k)
    // to ignore the irrelavant difference
    // we first extract the signature
    // using a jagged 2D array to store signature
    // outer length = number of arguments,
    // inner length = number of bounds of that argument
    
    genST*** signature1 = st1->argumentsCount>0?calloc(st1->argumentsCount, sizeof(genST**)):NULL;
    genST*** signature2 = st1->argumentsCount>0?calloc(st1->argumentsCount, sizeof(genST**)):NULL;
    
    // if they dont have argument, only compare non-argument boundedness
    if(!signature1 && !signature2){
        // non-argument types are identifier-sensitive
        return compareUnorderedGenSTArrays(st1->generics->nestedCount, st1->generics->nested, st2->generics->nested, 1);
    }
    
    assert(signature1 && signature2);
    if(signature1 && signature2){
    
        bool unmatched = false;
        
        // store size of each 1D array in signature
        size_t* sizes = calloc(st1->argumentsCount, sizeof(size_t));
        assert(sizes);
        
        // seperate type bounds for arguments and non-arguments
        
        // detect status of argument boundedness
        bool* argBounds1 = calloc(st1->generics->nestedCount, sizeof(bool));
        bool* argBounds2 = calloc(st1->generics->nestedCount, sizeof(bool));
        assert(argBounds1 && argBounds2);
        
        // store non-argument boundedness
        genST** na1 = calloc(st1->generics->nestedCount, sizeof(genST*));
        genST** na2 = calloc(st1->generics->nestedCount, sizeof(genST*));
        assert(na1 && na2);
        
        // count boundedness corresponding to argument, and initialize subaray
        for(size_t i=0; i < st1->argumentsCount; i++){
            int count1 = 0, count2 = 0;
            
            // count boundedness
            for(size_t j=0; j < st1->generics->nestedCount; j++){
                if(areGenericsEqual(st1->generics->nested[j], st1->arguments[i]->type, 1)){
                    count1++;
                    argBounds1[j] = true;
                }
                if(areGenericsEqual(st2->generics->nested[j], st2->arguments[i]->type, 1)){
                    count2++;
                    argBounds2[j] = true;
                }
            }
            
            // early exit
            if(count1 != count2){
                unmatched = true;
                goto clear_and_return;
            }
            
            // record size of 1D array
            sizes[i] = count1;
            
            // create subarray
            signature1[i] = (genST**)calloc(count1, sizeof(genST*));
            signature2[i] = (genST**)calloc(count1, sizeof(genST*));
            assert(signature1[i] && signature2[i]);
            
            // insert bounds
            int k1 = 0, k2 = 0;
            for(size_t j=0; j < st1->generics->nestedCount; j++){
                if(areGenericsEqual(st1->generics->nested[j], st1->arguments[i]->type, 1)){
                    signature1[i][k1] = st1->generics->nested[j];
                    k1++;
                }
                if(areGenericsEqual(st2->generics->nested[j], st2->arguments[i]->type, 1)){
                    signature2[i][k2] = st2->generics->nested[j];
                    k2++;
                }
            }
        }
        
        // unmarked bounds goes into non-argument
        size_t k1 = 0, k2 = 0;
        for(size_t i=0; i < st1->generics->nestedCount; i++){
            if(!argBounds1[i]){
                na1[k1] = st1->generics->nested[i];
                k1++;
            }
            if(!argBounds2[i]){
                na2[k2] = st2->generics->nested[i];
                k2++;
            }
        }
        if(k1 != k2){
            unmatched = true;
            goto clear_and_return;
        }
        
        // non-argument bounds matches?
        // identifier-sensitive, mode=1
        if(!compareUnorderedGenSTArrays(k1, na1, na2, 1)){
            unmatched = true;
            goto clear_and_return;
        }
              
        // compare signature
        // this is the only identifier-insensitive case, mode=0
        for(size_t i=0; i < st1->argumentsCount; i++){
            if(sizes[i]==0){
                if(areGenericsEqual(st1->arguments[i]->type, st2->arguments[i]->type, 0)){
                    continue;
                }else{
                    unmatched = true;
                    goto clear_and_return;
                }
            }
            if(!compareUnorderedGenSTArrays(sizes[i], signature1[i], signature2[i], 0)){
                unmatched = true;
                goto clear_and_return;
            }
        }
        
clear_and_return:
        for(size_t k=0; k < st1->argumentsCount; k++){
            free(signature1[k]);
        }
        for(size_t k=0; k < st1->argumentsCount; k++){
            free(signature2[k]);
        }
        free(signature1);
        free(signature2);
        free(argBounds1);
        free(argBounds2);
        free(na1);
        free(na2);
        free(sizes);
        return !unmatched;
    }

    return true;
}

bool compareUnorderedGenSTArrays(size_t size, genST** arr1, genST** arr2, int mode){
    if(size==0){
        return true;
    }
    if(!arr1 && !arr2){
        return true;
    }
    if(!arr1 || !arr2){
        return false;
    }
    
    bool* record = calloc(size, sizeof(bool));
    assert(record);
    bool unmatched = false;
    
    for(size_t i=0; i<size; i++){
        bool matchCurrent = false;
        for(size_t j=0; j<size; j++){
            if(record[j]){
                continue;
            }
            if(areGenericsEqual(arr1[i], arr2[j], mode)){
                record[j] = true;
                matchCurrent = true;
                break;
            }
        }
        if(!matchCurrent){
            unmatched = true;
            break;
        }
    }
    free(record);
    return !unmatched;
}

vtable* attachVirtualTable(classSTManager* cstm, classST* st){
    if(!cstm || !st){
        return NULL;
    }
    vtable* vt = calloc(1, sizeof(vtable));
    assert(vt);
    vt->attachClass = st;
    st->virtualTable = vt;
    
    // if current class is Object, then create the base vtable
    if(!strcmp("Object",st->generics->type)){
        //printf("%sDebug: is Object%s\n", RED, NRM);
        int count = 0;
        for(size_t i=0; i < st->methodsCount; i++){
            if(isVirtualMethod(st->methods[i])){
                count++;
            }
        }
        vt->entryCount = count;
        vt->entries = calloc(vt->entryCount, sizeof(methodST*));
        
        int k = 0;
        for(size_t i=0; i<st->methodsCount; i++){
            if(isVirtualMethod(st->methods[i])){
                vt->entries[k] = st->methods[i];
                k++;
            }
        }
        return vt;
    }
    
    // current class is not Object, extends superclass's vtable
    classST* super = lookupClassST(cstm, st->superclassGenerics->type);
    if(!super){
        fprintf(stderr, "%sError attachVirtualTable: superclass %s of %s is not in the registered table%s\n", RED, st->superclassGenerics->type, st->generics->type, NRM);
        exit(1);
    }
    vt->super = super->virtualTable;
    
    // record which method overrides theclassSymbolTable method from superclass, alleviate another loop
    // set default value = -1.
    int* record = calloc(super->virtualTable->entryCount, sizeof(int));
    int tmp = -1;
    memcpy(record, &tmp, sizeof(int)*super->virtualTable->entryCount);
    
    // record current st's virtual methods that does override
    bool* nov = calloc(st->methodsCount, sizeof(bool));
    
    // traverse all virtual methods from current class, if not override, count++
    int count = 0;
    for(size_t i=0; i < st->methodsCount; i++){
        if(!isVirtualMethod(st->methods[i])){
            if(hasInadequateOverride(st->methods[i])){
                fprintf(stderr, "%sError attachVirtualTable: method %s does not override superclass's method but has override annotation%s\n", RED, st->methods[i]->generics->type, NRM);
                //exit(1);
            }else{
                continue;
            }
        }
        nov[i] = true;
        bool overrides = false;
        for(size_t j=0; j < super->virtualTable->entryCount; j++){
            if(strcmp(st->methods[i]->generics->type, super->virtualTable->entries[j]->generics->type))
            {
                if(hasInadequateOverride(st->methods[i])){
                    fprintf(stderr, "%sError attachVirtualTable: method %s does not override superclass's method but has override annotation%s\n", RED, st->methods[i]->generics->type, NRM);
                    //exit(1);
                }else{
                    continue;
                }
            }else{
                if(methodOverrides(st->methods[i], super->virtualTable->entries[i])){
                    if(isValidOverrideReturnType(st->methods[i], super->virtualTable->entries[j], cstm) && st->methods[i]->access>=super->virtualTable->entries[j]->access){
                        overrides = true;
                        record[j] = i;
                        nov[i] = false;
                        continue;
                    }else{
                        fprintf(stderr, "%sError attachVirtualTable: trying to overrides %s but curent method have smaller accessibility or larger return type.%s\n", RED, super->virtualTable->entries[i]->generics->type, NRM);
                        exit(1);
                    }
                }else{
                    if(hasInadequateOverride(st->methods[i])){
                        fprintf(stderr, "%sError attachVirtualTable: method %s does not override superclass's method but has override annotation%s\n", RED, st->methods[i]->generics->type, NRM);
                        //exit(1);
                    }else{
                        continue;
                    }
                }
            }
        }
        if(!overrides){
            count++;
        }
    }
    
    vt->entryCount = count + super->virtualTable->entryCount;
    vt->entries = (methodST**)calloc(vt->entryCount, sizeof(methodST*));

    // copy all methods from superclass
    memcpy(vt->entries, super->virtualTable->entries, sizeof(methodST*)*super->virtualTable->entryCount);
    // then try to overwrite them, if overridden
    for(size_t i=0; i < super->virtualTable->entryCount; i++){
        if(record[i] != -1){
            vt->entries[i] = st->methods[record[i]];
        }
    }
    // insert non-overriding virtual methods
    int k = super->virtualTable->entryCount;
    for(size_t i=0; i < st->methodsCount; i++){
        if(nov[i]){
            vt->entries[k] = st->methods[i];
            k++;
        }
    }
    
    free(nov);
    free(record);
    return vt;
}

bool isValidOverrideReturnType(methodST* st1, methodST* st2, classSTManager* cstm){
    // for simplicity, return true if both returns primitive types
    if(st1->returnsPrimitive && st2->returnsPrimitive){
        return true;
    }
    if(st1->returnsPrimitive || st2->returnsPrimitive){
        return false;
    }
    
    // for reference type, method1 and method2 can return class or interface, leading to 4 branches
    // special case: if they have equal return type
    if(!strcmp(st1->returnType->type, st2->returnType->type)){
        return true;
    }
    
    // branch1 -- if method1 returns interface while method2 returns class: invalid
    if(st1->parentClass->classSymbolTable->isInterface && st2->parentClass->classSymbolTable->isClass){
        return false;
    }
    
    // branch2&3 -- if method2 returns interface: trace back implemented interfaces
    if(st1->parentClass->classSymbolTable->isInterface && st2->parentClass->classSymbolTable->isInterface){
        for(size_t i=0; i < st1->parentClass->classSymbolTable->interfacesCount; i++){
            classST* implInter = lookupClassST(cstm, st1->parentClass->classSymbolTable->interfacesGenerics[i]->type);
            if(!implInter){
                continue;
            }
            if(!strcmp(implInter->generics->type, st2->returnType->type)){
                return true;
            }
            if(isCompatibleInterface(implInter, lookupClassST(cstm, st2->parentClass->classSymbolTable->generics->type), cstm)){
                return true;
            }
        }
    return false;
    }
    
    // branch4 -- if both return classes. trace back superclass until Object
    // special case: if method2 returns Object
    if(!strcmp("Object", st2->returnType->type)){
        return true;
    }
    // common case
    classST* superclass = lookupClassST(cstm, st1->parentClass->classSymbolTable->superclassGenerics->type);
    // if superclass=NULL, it could be either imcompatible return types, or superclass not included in the registered table.
    while(superclass){
        if(!strcmp(superclass->generics->type, st2->returnType->type)){
            return true;
        }
        superclass = lookupClassST(cstm, superclass->superclassGenerics->type);
    }
    return false;
}

bool isCompatibleInterface(classST* st1, classST* st2, classSTManager* cstm){
    if(!st1 || !st2 || !cstm){
        return false;
    }
    for(size_t i=0; i < st1->interfacesCount; i++){
        classST* implInter = lookupClassST(cstm, st1->interfacesGenerics[i]->type);
        if(!implInter){
            return false;
        }
        if(!strcmp(implInter->generics->type, st2->generics->type)){
            return true;
        }
        if(isCompatibleInterface(implInter, st2, cstm)){
            return true;
        }
    }
    return false;
}

bool hasInadequateOverride(methodST* st){
    if(!st){
        return false;
    }
    if(!st->annotation){
        return true;
    }
    if(!strcmp(st->annotation, "Override")){
        return true;
    }
    return false;
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
    if(n->ruleType == subroutineDeclaration_rule || n->ruleType == subroutinePrototype_rule){
        printMethodST(n->methodSymbolTable);
        flag = true;
    }
    if(n->ruleType == variableDeclaration_rule){
        for(size_t i=0; i < n->varCount; i++){
            printVarST(n->varSymbolTable[i]);
        }
        flag = true;
    }
    if(n->ruleType == ifStatement_rule || n->ruleType == switchStatement_rule ||
       n->ruleType == forStatement_rule || n->ruleType == whileStatement_rule ||
       n->ruleType == doWhileStatement_rule || n->ruleType == staticStatement_rule ||
       n->ruleType == codeBlock_rule
    ){
       
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

    
    printf(", returnType =");
    if(!st->returnType){
        printf(" constructor");
    }else{
        printGenericsST(st->returnType);
    }
    
    if(st->arrDimension>0){
        printf(", array dimension = %ld", st->arrDimension);
    }
    
    printf(", type boundedness =");
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
    if(st->virtualTable && st->virtualTable->entryCount>0){
        free(st->virtualTable->entries);
    }
    free(st->virtualTable);
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

void freeCSTM(classSTManager* cstm){
    if(!cstm){
        return;
    }
    free(cstm->registeredTables);
    free(cstm);
}

void* testcalloc(size_t len, size_t size){
    return calloc(len, size);
}
