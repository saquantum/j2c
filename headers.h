#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<ctype.h>
#include<assert.h>
#include<time.h>

#define NRM "\033[0m"
#define RED "\033[1;31m"
#define YLW "\033[1;33m"
#define GRN "\033[1;32m"

struct tokenTable;
struct CST;

typedef struct resourceManager{
    struct tokenTable** tables;
    struct CST** trees;
    size_t count;
    size_t capacity;
}resourceManager;

typedef struct stack{
    void** stack;
    size_t length;
    size_t capacity;
}stack;

char* mystrdup(char* str);
int hash_djb2(char* str);

stack* stack_create();
bool stack_push(stack* s, void* p);
void* stack_pop(stack* s);
void* stack_peektop(stack* s);
void stack_free(stack* s);
