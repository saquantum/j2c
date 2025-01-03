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
