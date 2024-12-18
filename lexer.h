#pragma once

#include "token.h"

typedef struct hll{
    char buffer[256];
    int current;
    struct hll* next;
} hll;

tokenTable* lexFile(FILE* f);

token* lexToken(char* str);

hll* createHLL();

void insert2HLL(char c, hll* h);

char* hll2str(hll* h);

void freeHLL(hll** h);
