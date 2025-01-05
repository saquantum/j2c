#include "symboltable.h"

void readSystem(classSTManager* cstm);

int main(int argc, char** argv){
    assert(argc == 3);
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    
    classSTManager* cstm = createCSTM();
    
    // import Object
    FILE* inputObject = fopen("./system/Object.java", "r");
    assert(inputObject);
    tokenTable* ttableObject = lexFile(inputObject);
    combineSymbols(ttableObject);
    CST* cstObject = parseTokenTable("./system/Object.java", ttableObject);
    attachSymbolTables(cstObject);
    insertClass2CSTM(cstm, cstObject);
    fclose(inputObject);
    
    
    tokenTable* ttable = lexFile(input);
    combineSymbols(ttable);
    
    //printTokenTable(ttable);
    printf("--------------\n");

    CST* cst = parseTokenTable(argv[1], ttable);
    
    printLessCST(cst);
    printf("--------------\n");
    
    attachSymbolTables(cst);
    printSymbolTables(cst);
    
    
    fclose(input);
    fclose(output);
    
    
    freeCSTM(cstm);
    freeSymbolTables(cst);
    freeCST(&cst);
    freeTokenTable(&ttable);
    
    freeSymbolTables(cstObject);
    freeCST(&cstObject);
    freeTokenTable(&ttableObject);
}
   
    

