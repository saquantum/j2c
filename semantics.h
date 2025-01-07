//#pragma once
#include "symboltable.h"

bool checkCSTM(classSTManager* cstm);

bool checkClassInheritance(classSTManager* cstm, classST* st);
bool checkInterfaceImplementation(classSTManager* cstm, classST* st);
bool checkCyclicImplementation(classSTManager* cstm, classST* st, classST* current);
bool checkAbstract(classSTManager* cstm,classST* st);
