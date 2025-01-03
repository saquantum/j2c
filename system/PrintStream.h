#pragma once
#include "Object.h"
#include "String.h"

struct Object$obj;
struct String$obj;

struct PrintStream$obj{
    struct Object$obj super;
};


void PrintStream$printf$0(struct String$obj* s);
