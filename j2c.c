#include "semantics.h"

void processFile(classSTManager* cstm, resourceManager* rm, char* filepath);
resourceManager* createResourceManager();
void insert2RM(resourceManager* rm, tokenTable* table, CST* cst);
void freeRM(resourceManager* rm);

int main(int argc, char** argv){
    assert(argc == 3);
    FILE* input = fopen(argv[1], "r");
    FILE* output = fopen(argv[2], "w");
    
    resourceManager* rm = createResourceManager();
    
    classSTManager* cstm = createCSTM();
    
    // import system files
    processFile(cstm, rm, "./system/Object.java");
    processFile(cstm, rm, "./system/Comparable.java");
    processFile(cstm, rm, "./system/String.java");
    processFile(cstm, rm, "./system/Array.java");
    
    // main compile
    tokenTable* ttable = lexFile(input);
    combineSymbols(ttable);
    
    //printTokenTable(ttable);
    //printf("--------------\n");

    CST* cst = parseTokenTable(argv[1], ttable);
    
    //printLessCST(cst);
    //printf("--------------\n");
    
    attachSymbolTables(cst);
    insertClass2CSTM(cstm, cst);
    printSymbolTables(cst);
    
    
    printf("%s--------------------------%s\n", RED, NRM);
    printCSTM(cstm);
    
    assert(checkCSTM(cstm));
    
    fclose(input);
    fclose(output);
    
    insert2RM(rm, ttable, cst);
    
    freeCSTM(cstm);
    
    freeRM(rm);
}

void processFile(classSTManager* cstm, resourceManager* rm, char* filepath){
    FILE* input = fopen(filepath, "r");
    assert(input);
    tokenTable* table = lexFile(input);
    combineSymbols(table);
    CST* cst = parseTokenTable(filepath, table);
    attachSymbolTables(cst);
    insertClass2CSTM(cstm, cst);
    fclose(input);
    insert2RM(rm, table, cst);
}
   
resourceManager* createResourceManager(){
    resourceManager* rm =calloc(1, sizeof(resourceManager));
    assert(rm);
    rm->capacity = 16;
    rm->tables = (tokenTable**)calloc(16, sizeof(tokenTable*));
    rm->trees = (CST**)calloc(16, sizeof(CST*));
    return rm;
}

void insert2RM(resourceManager* rm, tokenTable* table, CST* cst){
    if(rm->count == rm->capacity){
        rm->capacity *= 2;
        rm->tables = realloc(rm->tables, rm->capacity*sizeof(tokenTable*));
        rm->trees = realloc(rm->trees, rm->capacity*sizeof(CST*));
        assert(rm->tables && rm->trees);
    }
    rm->tables[rm->count] = table;
    rm->trees[rm->count] = cst;
    rm->count++;
}

void freeRM(resourceManager* rm){
    for(size_t i=0; i<rm->count; i++){
        freeSymbolTables(rm->trees[i]);
        freeCST(&rm->trees[i]);
        freeTokenTable(&rm->tables[i]);
    }
    free(rm->trees);
    free(rm->tables);
    free(rm);
}

