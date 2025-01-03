#pragma once
#include "Object.h"
#include "PrintStream.h"

struct Object$obj;
struct PrintStream$obj;

struct System{
    struct Object$obj* super;
    struct PrintStream$obj* out;
};

void System$exit$0(int e);

long System$currentTimeMillis$0();
