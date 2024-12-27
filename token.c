#include "token.h"

char* getKeyword(keyword KEY){
    static char* keywords[] = {
        "char", "int", "long", "boolean", "double",
        "for", "while", "do", "if", "else", "switch", "case", "default", "continue", "break", "return", 
        "public", "private", 
        "static", "final", 
        "true", "false", 
        "null", "void",
        "import", 
        "try", "catch", "finally", "throw", "throws", 
        "class", "abstract", "interface", 
        "extends", "implements", 
        "this", "super", 
        "new", 
        "instanceof", 
        "native"};
    return keywords[KEY];
}

int isKeyword(char* str){
    static char* keywords[] = {
        "char", "int", "long", "boolean", "double",
        "for", "while", "do", "if", "else", "switch", "case", "default", "continue", "break", "return", 
        "public", "private", 
        "static", "final", 
        "true", "false", 
        "null", "void",
        "import", 
        "try", "catch", "finally", "throw", "throws", 
        "class", "abstract", "interface", 
        "extends", "implements", 
        "this", "super", 
        "new", 
        "instanceof", 
        "native"};
    for(int i=0;i<(int)(sizeof(keywords)/sizeof(keywords[0]));i++){
        if(!strcmp(str, keywords[i])){
            return i;
        }
    }
    return -1;
}

token* createToken(){
    return (token*)calloc(1,sizeof(token));
}

void freeToken(token* t){
    if(t->type == IDENTIFIER || t->type == STRING || t->type == NUMBER || t->type == CHARACTER || t->type == OPERATOR){
        free(t->data.str_val);
    }
    free(t);
}

tokenTable* createTokenTable(){
    return (tokenTable*)calloc(1,sizeof(tokenTable));
}

void printTokenTable(tokenTable* table){
    if(!table){
        printf("Empty or null token table detected.\n");
        return;
    }
    
    tokenNode* n = table->start;
    while(n){
        switch(n->t->type){
            case KEYWORD:
                printf("TokenType = Keyword, TokenValue = %s, lineNumber = %d\n", getKeyword(n->t->data.key_val), n->t->lineNumber);
                break;
            case NUMBER:
                printf("TokenType = Number, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case CHARACTER:
                printf("TokenType = Character, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case IDENTIFIER:
                printf("TokenType = Identifier, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case OPERATOR:
                printf("TokenType = Operator, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case STRING:
                printf("TokenType = String, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case SYMBOL:
                printf("TokenType = Symbol, TokenValue = %c, lineNumber = %d\n", n->t->data.char_val, n->t->lineNumber);
                break;
            case BRACKET:
                printf("TokenType = Bracket, TokenValue = %c, lineNumber = %d\n", n->t->data.char_val, n->t->lineNumber);
                break;
            case SEMICOLON:
                printf("TokenType = Semicolon, TokenValue = %c, lineNumber = %d\n", n->t->data.char_val, n->t->lineNumber);
                break;
        }
        
        n = n->next;
    }
}

void printCurrentToken(tokenNode* n){
    if(!n){
        printf("Currently has reached end of the table.");
        return;
    }
    printf("Current token: ");
        switch(n->t->type){
            case KEYWORD:
                printf("TokenType = Keyword, TokenValue = %s, lineNumber = %d\n", getKeyword(n->t->data.key_val), n->t->lineNumber);
                break;
            case NUMBER:
                printf("TokenType = Number, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case CHARACTER:
                printf("TokenType = Character, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case IDENTIFIER:
                printf("TokenType = Identifier, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case OPERATOR:
                printf("TokenType = Operator, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case STRING:
                printf("TokenType = String, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case SYMBOL:
                printf("TokenType = Symbol, TokenValue = %c, lineNumber = %d\n", n->t->data.char_val, n->t->lineNumber);
                break;
            case BRACKET:
                printf("TokenType = Bracket, TokenValue = %c, lineNumber = %d\n", n->t->data.char_val, n->t->lineNumber);
                break;
            case SEMICOLON:
                printf("TokenType = Semicolon, TokenValue = %c, lineNumber = %d\n", n->t->data.char_val, n->t->lineNumber);
                break;
        }
}

bool writeToken(token* t, tokenTable* table){
    if(!t || !table){
        return false;
    }
    
    tokenNode* node = calloc(1,sizeof(tokenNode));
    if(!node){
        fprintf(stderr, "Error writeToken: not enough heap memory, cannot create token node.\n");
        exit(1);
    }
    node->t = t;
    if(!table->start){
        table->start = node;
        table->current = node;
    }else{
        table->end->next = node;
        node->prev = table->end;
    }
    table->end = node;
    return true;
}

bool hasNext(tokenTable* table){
    if (!table || !table->current || !table->start){
        return false;
    }
    return true;
}

tokenNode* nextNode(tokenTable* table){
    if (!table || !table->current || !table->start){
        return NULL;
    }
    tokenNode* tmp = table->current;
    table->current = table->current->next;
    return tmp;
}

void unread(tokenTable* table){
    if(!table){
        return;
    }
    if(!table->current){
        table->current = table->end;
        return;
    }
    if(table->current == table->start){
        return;
    }
    table->current = table->current->prev;
}

tokenNode* peekNextNode(tokenTable* table){
    if (!table || !table->start){
        return NULL;
    }
    return table->current;
}

tokenNode* prevNode(tokenTable* table){
    if (!table || !table->start){
        return NULL;
    }
    if(!table->current){
        return table->end;
    }
    return table->current->prev;
}

void combineSymbols(tokenTable* table){
    if(!table || !table->start){
        return;
    }
    tokenNode* current = table->start;
    while(current && current->next){
        if(current->t->type==SYMBOL && current->next->t->type==SYMBOL){
            char combined[3] = {current->t->data.char_val, current->next->t->data.char_val, 0};
            if(!strcmp(combined, "&&") || !strcmp(combined, "||") || !strcmp(combined, "==") || !strcmp(combined, "!=") || !strcmp(combined, "+=") || !strcmp(combined, "-=") || !strcmp(combined, "*=") || !strcmp(combined, "/=")  || !strcmp(combined, "++")  || !strcmp(combined, "--") ){
                current->t->type = OPERATOR;
                current->t->data.str_val = calloc(3,sizeof(char));
                if(!current->t->data.str_val){
                    fprintf(stderr, "Error combineSymbols: not enough memory, cannot create a string value for the node\n");
                    exit(1);
                }
                strcpy(current->t->data.str_val, combined);
                tokenNode* tmp = current->next;
                current->next = tmp->next;
                if(tmp->next){
                    tmp->next->prev = current;
                }else{
                    table->end = current;
                }
                freeToken(tmp->t);
                free(tmp);
            }
        }
        current = current->next;
    }
}

void freeTokenTable(tokenTable** table){
    tokenNode* n = (*table)->start;
    
    while(n){
        tokenNode* tmp = n->next;
        freeToken(n->t);
        free(n);
        n = tmp;
    }
        
    free(*table);
    *table=NULL;
}
