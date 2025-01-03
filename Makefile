#! /usr/bin/env bash

test: j2c.c symboltable.c symboltable.h parser.c parserhelper.c parser.h lexer.c token.c j2ctools.c headers.h lexer.h token.h 
	gcc j2c.c symboltable.c parser.c parserhelper.c lexer.c token.c j2ctools.c -Wno-error=unused-parameter -Wall -Wextra -Wpedantic -Wvla -Wfloat-equal -fsanitize=address -fsanitize=undefined -Werror -o j2c -lm
	
parse: j2c.c parser.c parserhelper.c parser.h lexer.c token.c j2ctools.c headers.h lexer.h token.h 
	gcc j2c.c parser.c parserhelper.c lexer.c token.c j2ctools.c -Wno-error=unused-parameter -Wall -Wextra -Wpedantic -Wvla -Wfloat-equal -fsanitize=address -fsanitize=undefined -Werror -o j2c -lm
