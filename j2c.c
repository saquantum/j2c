#include "parser.h"

int main(int argc, char** argv){
    assert(argc == 3);
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    
    tokenTable* ttable = lexFile(input);
    combineSymbols(ttable);
    
    printTokenTable(ttable);
    printf("--------------\n");

    CST* cst = parseTokenTable(argv[1], ttable);
    
    printLessCST(cst);
    printf("--------------\n");
    
    
    fclose(input);
    fclose(output);
    freeCST(&cst);
    freeTokenTable(&ttable);
    
}
