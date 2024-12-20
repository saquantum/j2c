#include "token.h"

token* createToken(){
    return (token*)calloc(1,sizeof(token));
}

void freeToken(token* t){
    if(t->type == IDENTIFIER || t->type == STRING || t->type == NUMBER){
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
    char* keywords[] = {"char", "int", "long", "boolean", "double", "for", "while", "do", "if", "else", "switch", "case", "default", "continue", "break", "return", "public", "private", "static", "final", "true", "false", "null", "import", "try", "catch", "finally", "throw", "throws", "class", "abstract", "interface", "extends", "implements", "this", "that", "new", "instanceof", "native"};
    tokenNode* n = table->start;
    while(n){
        switch(n->t->type){
            case KEYWORD:
                printf("TokenType = Keyword, TokenValue = %s, lineNumber = %d\n", keywords[n->t->data.key_val], n->t->lineNumber);
                break;
            case NUMBER:
                printf("TokenType = Number, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
                break;
            case IDENTIFIER:
                printf("TokenType = Identifier, TokenValue = %s, lineNumber = %d\n", n->t->data.str_val, n->t->lineNumber);
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

tokenNode* peekNextNode(tokenTable* table){
    if (!table || !table->current || !table->start){
        return NULL;
    }
    return table->current->next;
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
