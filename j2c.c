#include "parser.h"

int main(int argc, char** argv){
    assert(argc == 3);
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    
    tokenTable* ttable = lexFile(input);
    combineSymbols(ttable);
    
    printTokenTable(ttable);
    
    fclose(input);
    fclose(output);
    freeTokenTable(&ttable);
}
