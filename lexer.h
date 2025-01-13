#pragma once

#include "token.h"

typedef struct hll{
    char buffer[256];
    size_t current;
    struct hll* next;
} hll;

void resetLineNumber();

tokenTable* lexFile(FILE* f);

token* lexToken(char* str, int lineNumber);

bool isValidChar(char* str);

hll* createHLL();

void insert2HLL(char c, hll* h);

char* hll2str(hll* h);

void freeHLL(hll** h);
