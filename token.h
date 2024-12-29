#pragma once
#include "headers.h"

typedef enum tokenType{
    KEYWORD,
    NUMBER,
    CHARACTER,
    IDENTIFIER,
    OPERATOR,
    STRING,
    SYMBOL,
    BRACKET,
    SEMICOLON 
} tokenType; 

typedef enum keyword{
    CHAR, INT, LONG, BOOLEAN, DOUBLE,
    FOR, WHILE, DO, IF, ELSE, SWITCH, CASE, DEFAULT, CONTINUE, BREAK, RETURN,
    PUBLIC, PRIVATE,
    STATIC, FINAL, 
    BOOL_TRUE, BOOL_FALSE, 
    NULLER, VOID,
    IMPORT,
    TRY, CATCH, FINALLY, THROW, THROWS,
    CLASS, ABSTRACT, INTERFACE,
    EXTENDS, IMPLEMENTS, 
    THIS, SUPER,
    NEW,
    INSTANCEOF,
    NATIVE
} keyword;

typedef union tokenData{
    keyword key_val;
    char char_val; // this includes symbols, delimiters and newline
    char* str_val; // this includes numbers, strings, operators and identifiers. deal with number later with a parser. since string literal must have \" surroundings, we can distinguish them with other identifiers, among which starts with a digit, a dot or a minus sign is a number then.
} tokenData;

typedef struct token{
    tokenType type;
    tokenData data;
    int lineNumber;
} token;

// store all tokens in a linked list

typedef struct tokenNode{
    token* t;
    struct tokenNode* prev;
    struct tokenNode* next;
} tokenNode;

typedef struct tokenTable{
    tokenNode* start;
    tokenNode* current;
    tokenNode* end;
} tokenTable;

char* mystrdup(char* str);

char* getKeyword(keyword KEY);
int isKeyword(char* str);

token* createToken();
void freeToken(token* t);

tokenTable* createTokenTable();
void printTokenTable(tokenTable* table);

void printCurrentToken(tokenNode* n);

// store token in token table
bool writeToken(token* t, tokenTable* table);

// table has more nodes?
bool hasNext(tokenTable* table);

// read the next node, and move current pointer forward.
tokenNode* nextNode(tokenTable* table);

// move current pointer backward.
void unread(tokenTable* table);

// read the next node, but does not move current pointer forward.
tokenNode* peekNextNode(tokenTable* table);

// read the previous node. NOTICE: this does not move pointer current.
tokenNode* prevNode(tokenTable* table);

// sweep the token table to combine symbols into two-symbol operators
// NOTICE: this preprocessing combiner ignores '<' and '>' ,
// since we must use grammar and context to determine
// whether they represent generics or operators
// we will defer this until parsing.
void combineSymbols(tokenTable* table);

void freeTokenTable(tokenTable** table);
