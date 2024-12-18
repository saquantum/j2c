#include "token.h"

token* createToken(){
    return (token*)calloc(1,sizeof(token));
}

void freeToken(token* t){
    if(t->type == IDENTIFIER || t->type == NUMBER){
        free(t->data.str_val);
    }
    free(t);
}

tokenTable* createTokenTable(){
    return (tokenTable*)calloc(1,sizeof(tokenTable));
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
