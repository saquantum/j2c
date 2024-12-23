#include "parser.h"

bool isExpressionStart(token* t){
    if(!t){
        return false;
    }
    return t->type==NUMBER || t->type==STRING || t->type==IDENTIFIER ||
            (t->type==KEYWORD && (
            t->data.key_val==BOOL_TRUE || t->data.key_val==BOOL_FALSE ||
            t->data.key_val==NULLER || t->data.key_val==THIS
            ) ) ||
            (t->type==SYMBOL && (
            t->data.char_val=='!' || t->data.char_val=='-' ||
            t->data.char_val=='~'
            ) ) ||
            (t->type==BRACKET && t->data.char_val=='(');
}

bool isKey(keyword Key, tokenNode* n){
    return n && n->t->type==KEYWORD && n->t->data.key_val==Key;
}
bool isSymbol(char c, tokenNode* n){
    return n && n->t->type==SYMBOL && n->t->data.char_val==c;
}
bool isIdentifier(tokenNode* n){
    return n && n->t->type==IDENTIFIER;
}
bool isOperator(char* o, tokenNode* n){
    return n && n->t->type==OPERATOR && !strcmp(o, n->t->data.str_val);
}
bool isBracket(char c, tokenNode* n){
    return n && n->t->type==BRACKET && n->t->data.char_val==c;
}
bool isNumber(tokenNode* n){
    return n && n->t->type==NUMBER;
}
bool isString(tokenNode* n){
    return n && n->t->type==STRING;
}
bool isSemicolon(tokenNode* n){
    return n && n->t->type==SEMICOLON;
}

bool isPotentialGenerics(tokenNode* current){
    // Tentatively treat '<' or '>' as part of generics
    // List<>
    // List<String>
    // Map<List<?>, List<>>
    // Map<?, List<>>
    // Map<List<String>, List<String>>
    // Map<Map<List<? extends Iterable>, List<>>, Map<List<>, List<>>>
    
    // cracking rules (version YZH):
    // <: identifier, '<', (identifier | '?' | '>') 
    // >: (identifier | '?' | '<' | '>'), '>', (',' | '>')  (depth>0)
    // ,: ('>' | '?' | identifier), ',', (identifier | '?')
    // ?: (',' | '<'), '?', (extends | super | ',' | '>')
    // identifier: (extends | super | ',' | '<'), identifier, ('<' | '>' | ',')
    
    int depth = 0; // mimics stack
    while (current) {
        //printCurrentToken(current);
        if(isSymbol('<', current)){
            depth++;
            if(isIdentifier(current->prev) && (isIdentifier(current->next) || isSymbol('?', current->next) || isSymbol('>', current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid '<' detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        if(isSymbol('>', current)){
            depth--;
            if(depth==0){
                return true;
            }
            // depth > 0
            if( (isIdentifier(current->prev) || isSymbol('?', current->prev) || isSymbol('<', current->prev) || isSymbol('>', current->prev)) && (isSymbol(',', current->next) || isSymbol('>', current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid '>' detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        if(isSymbol(',', current)){
            if( (isIdentifier(current->prev) || isSymbol('?', current->prev) || isSymbol('>', current->prev)) && (isSymbol('?', current->next) || isIdentifier(current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid ',' detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        if(isSymbol('?', current)){
            if( (isSymbol('<', current->prev) || isSymbol(',', current->prev)) ){
                if(isSymbol('?', current->next) || isSymbol('>', current->next)){
                    current = current->next;
                    continue;
                }
                if(isKey(EXTENDS, current->next) || isKey(SUPER, current->next)){
                    current = current->next->next;
                    continue;
                }
            }
            //printf("Debug: invalid '?' detected. Its previous and next nodes are:\n");
            //printCurrentToken(current->prev);
            //printCurrentToken(current->next);
            return false;
        }
        if(isIdentifier(current)){
            if( (isKey(EXTENDS, current->prev) || isKey(SUPER, current->prev) || isSymbol(',', current->prev) || isSymbol('<', current->prev)) && (isSymbol(',', current->next) || isSymbol('<', current->next) || isSymbol('>', current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid identifier detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        //printf("Debug: invalid unknown token detected.\n");
        break; // invalid token
    }
    
    return false;
}

void checkKeyValueNodeExpected(tokenNode* n, tokenType expectedType, keyword expectedValue, char* functionName, char* errorMessage){
    if(!n || n->t->type != expectedType || n->t->data.key_val != expectedValue){
        fprintf(stderr, "Error %s line %d: %s.\n", functionName, n->t->lineNumber, errorMessage);
        exit(1);
    }
}

void checkCharValueNodeExpected(tokenNode* n, tokenType expectedType, char expectedValue, char* functionName, char* errorMessage){
    if(expectedValue == -1){
        if(!n || n->t->type != expectedType){
            fprintf(stderr, "Error %s line %d: %s.\n", functionName, n->t->lineNumber, errorMessage);
            exit(1);
        }
    }
    else{
        if(!n || n->t->type != expectedType || n->t->data.char_val != expectedValue){
            fprintf(stderr, "Error %s line %d: %s.\n", functionName, n->t->lineNumber, errorMessage);
            exit(1);
        }
    }
}

void checkStringValueNodeExpected(tokenNode* n, tokenType expectedType, char* expectedValue, char* functionName, char* errorMessage){
    if(expectedValue == NULL){
        if(!n || n->t->type != expectedType){
            fprintf(stderr, "Error %s line %d: %s.\n", functionName, n->t->lineNumber, errorMessage);
            exit(1);
        }
    }
    else{
        if(!n || n->t->type != expectedType || strcmp(n->t->data.str_val, expectedValue)){
            fprintf(stderr, "Error %s line %d: %s.\n", functionName, n->t->lineNumber, errorMessage);
            exit(1);
        }
    }
}

treeNode* insertNewNode2Parent(char* rule, token* t, treeNode* parent){
    treeNode* child = createTreeNode(rule, t);
    child->parent = parent;
    insertChildNode(parent, child);
    return child;
}

treeNode* createTreeNode(char* rule, token* t){
    if(!rule){
        fprintf(stderr, "Error createTreeNode: null rule provided.\n");
        exit(1);
    }
    treeNode* n = calloc(1,sizeof(treeNode));
    if(!n){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create tree node.\n", t->lineNumber);
        exit(1);
    } 
    n->ruleType = calloc((int)strlen(rule)+1, sizeof(char));
    if(!n->ruleType){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create rule type string.\n", t->lineNumber);
        exit(1);
    } 
    strcpy(n->ruleType, rule);
    n->assoToken = t;
    n->children = calloc(16, sizeof(treeNode*));
    if(!n->children){
        fprintf(stderr, "Error createTreeNode line %d: not enough memory, cannot create children array.\n", t->lineNumber);
        exit(1);
    } 
    n->capacity = 16;
    return n;
}

void insertChildNode(treeNode* n, treeNode* child){
    if(!n || !child){
        fprintf(stderr, "Error insertChildNode: null pointer provided.\n");
        exit(1);
    }
    if(n->capacity < n->childCount){
        if(n->assoToken){
            fprintf(stderr, "Error insertChildNode line %d: code of parser has destructively wrong logic.\n", n->assoToken->lineNumber);
        }else{
            fprintf(stderr, "Error insertChildNode: code of parser has destructively wrong logic.\n");
        }
        exit(1);
    }
    if(n->capacity == n->childCount){
        n->children = (treeNode**)realloc(n->children, 2*(n->capacity)*sizeof(treeNode*));
        if(!n->children){
            if(n->assoToken){
                fprintf(stderr, "Error insertChildNode line %d: not enough memory, cannot realloc children array.\n", n->assoToken->lineNumber);
            }else{
                fprintf(stderr, "Error insertChildNode: not enough memory, cannot realloc children array.\n");
            }
            exit(1);
        }
        n->capacity = 2 * (n->capacity);
    }
    n->children[n->childCount++] = child;
    child->parent = n;
}

void printCST(CST* cst){
    if(!cst){
        return;
    }
    printTreeNode(cst->root);
}

void printTreeNode(treeNode* n){
    if(!n){
        return;
    }
    if(!n->assoToken){
        printf("Rule = %s", n->ruleType);
    }
    else{
        switch(n->assoToken->type){
            case KEYWORD:
                printf("TokenType = Keyword, TokenValue = %s, lineNumber = %d", getKeyword(n->assoToken->data.key_val), n->assoToken->lineNumber);
                break;
            case NUMBER:
                printf("TokenType = Number, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case IDENTIFIER:
                printf("TokenType = Identifier, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case OPERATOR:
                printf("TokenType = Operator, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case STRING:
                printf("TokenType = String, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case SYMBOL:
                printf("TokenType = Symbol, TokenValue = %c, lineNumber = %d", n->assoToken->data.char_val, n->assoToken->lineNumber);
                break;
            case BRACKET:
                printf("TokenType = Bracket, TokenValue = %c, lineNumber = %d", n->assoToken->data.char_val, n->assoToken->lineNumber);
                break;
            case SEMICOLON:
                printf("TokenType = Semicolon, TokenValue = %c, lineNumber = %d", n->assoToken->data.char_val, n->assoToken->lineNumber);
                break;
        }
    }
    if(n->parent){
        printf(", ParentRule = %s", n->parent->ruleType);
    }
    if(n->childCount){
        printf(", ChildRules: ");
        for(int i=0; i<n->childCount; i++){
            printf("%s, ", n->children[i]->ruleType);
        }
    }
    printf(".\n");
    if(n->childCount){
        for(int i=0; i<n->childCount; i++){
            printTreeNode(n->children[i]); 
        }
        
    }
}

void freeCST(CST** cst){
    if(!cst || !(*cst)){
        return;
    }
    freeTreeNode((*cst)->root);
    free(*cst);
    *cst = NULL;
}

void freeTreeNode(treeNode* n){
    if(!n){
        return;
    }
    for(int i=0; i < n->childCount; i++){
        freeTreeNode(n->children[i]);
    }
    free(n->children);
    free(n->ruleType);
    free(n);
}
