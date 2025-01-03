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

char* mystrdup(char* str);
int hash_djb2(char* str);
