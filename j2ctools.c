#include "headers.h"

char* mystrdup(char* str){
    if(!str){
        return NULL;
    }
    char* s = calloc((int)strlen(str)+1, sizeof(char));
    if(!s){
        fprintf(stderr, "Error mystrdup: out of memory.\n");
        exit(1);
    }
    strcpy(s, str);
    return s;
}


int hash_djb2(char* str){
    if(!str){
        return 0;
    }
    
    int hash = 5381;
    int c;
    while((c = *str++)){
        hash = ((hash<<5) + hash) + c;
    }
    return hash;
}

stack* stack_create(){
    stack* s = calloc(1, sizeof(stack));
    assert(s);
    s->capacity = 16;
    s->stack = calloc(16, sizeof(void*));
    assert(s->stack);
    return s;
}

bool stack_push(stack* s, void* p){
    if(!s || !p){
        return false;
    }
    if(s->length == s->capacity){
        s->capacity *= 2;
        s->stack = realloc(s->stack, s->capacity*sizeof(void*));
        assert(s->stack);
    }
    s->stack[s->length++] = p;
    return true;
}

void* stack_pop(stack* s){
    if(!s || !s->length){
        return NULL;
    }
    return s->stack[--s->length];
}

void* stack_peektop(stack* s){
    if(!s || !s->length){
        return NULL;
    }
    return s->stack[s->length-1];
}

void stack_free(stack* s){
    if(!s){
        return;
    }
    free(s->stack);
    free(s);
}
