#! /usr/bin/env bash

test: j2c.c parser.c parserhelper.c parser.h lexer.c token.c headers.h lexer.h token.h 
	gcc j2c.c parser.c parserhelper.c lexer.c token.c -Wno-error=unused-parameter -Wall -Wextra -Wpedantic -std=c99 -Wvla -Wfloat-equal -fsanitize=address -fsanitize=undefined -Werror -o t -lm
	
