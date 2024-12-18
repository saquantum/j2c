#pragma once
#include "headers.h"

typedef enum tokenType{
    KEYWORD,
    NUMBER,
    IDENTIFIER,
    SYMBOL,
    BRACKET,
    SEMICOLON
} tokenType; 

typedef enum keyword{
    CHAR, INT, LONG, BOOLEAN, DOUBLE, UNSIGNED,
    FOR, WHILE, DO, IF, ELSE, SWITCH, CASE, DEFAULT, CONTINUE, BREAK, RETURN,
    PUBLIC, PRIVATE,
    STATIC, FINAL, 
    BOOL_TRUE, BOOL_FALSE, 
    NULLER,
    IMPORT,
    TRY, CATCH, FINALLY, THROW, THROWS,
    CLASS, ABSTRACT, INTERFACE,
    EXTENDS, IMPLEMENTS, 
    THIS, SUPER,
    NEW,
    INSTANCEOF
} keyword;

typedef union tokenData{
    keyword key_val;
    char char_val; // this includes symbols, delimiters and newline
    char* str_val; // this includes numbers and identifiers. deal with number later with a parser. since string literal must have \" surroundings, we can distinguish them with other identifiers, among which starts with a digit, a dot or a minus sign is a number then.
} tokenData;

typedef struct token{
    tokenType type;
    tokenData data;
} token;

// store all tokens in a linked list

typedef struct tokenNode{
    token* t;
    struct tokenNode* next;
} tokenNode;

typedef struct tokenTable{
    tokenNode* start;
    tokenNode* current;
    tokenNode* end;
} tokenTable;



token* createToken();

void freeToken(token* t);

tokenTable* createTokenTable();

void printTokenTable(tokenTable* table);

// store token in token table
bool writeToken(token* t, tokenTable* table);

// table has more nodes?
bool hasNext(tokenTable* table);

// read the next node
tokenNode* nextNode(tokenTable* table);

void freeTokenTable(tokenTable** table);
