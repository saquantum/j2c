#include "parser.h"

bool isKey(keyword Key, tokenNode* n){
    return n && n->t->type==KEYWORD && n->t->data.key_val==Key;
}
bool isSymbol(char c, tokenNode* n){
    return n && n->t->type==SYMBOL && n->t->data.char_val==c;
}
bool isIdentifier(tokenNode* n){
    return n && n->t->type==IDENTIFIER;
}
bool isOperator(char* o, tokenNode* n){
    return n && n->t->type==OPERATOR && !strcmp(o, n->t->data.str_val);
}
bool isBracket(char c, tokenNode* n){
    return n && n->t->type==BRACKET && n->t->data.char_val==c;
}
bool isNumber(tokenNode* n){
    return n && n->t->type==NUMBER;
}
bool isCharacter(tokenNode* n){
    return n && n->t->type==CHARACTER;
}
bool isString(tokenNode* n){
    return n && n->t->type==STRING;
}
bool isSemicolon(tokenNode* n){
    return n && n->t->type==SEMICOLON;
}

bool isExpressionStart(tokenNode* current){
    if(!current->t){
        return false;
    }
    return current->t->type==NUMBER || current->t->type==STRING || current->t->type==IDENTIFIER ||
            (current->t->type==KEYWORD && (
            current->t->data.key_val==BOOL_TRUE || current->t->data.key_val==BOOL_FALSE ||
            current->t->data.key_val==NULLER || current->t->data.key_val==THIS ||
            current->t->data.key_val==NEW
            ) ) ||
            (current->t->type==SYMBOL && (
            current->t->data.char_val=='!' || current->t->data.char_val=='-' ||
            current->t->data.char_val=='~'
            ) ) ||
            (current->t->type==OPERATOR && (
            !strcmp(current->t->data.str_val, "++") || 
            !strcmp(current->t->data.str_val, "--") 
            ) ) ||
            (current->t->type==BRACKET && current->t->data.char_val=='(') || 
            current->t->type==CHARACTER
            ;
}

bool isPotentialGenerics(tokenNode* current){
    // Tentatively treat '<' or '>' as part of generics
    // List<>
    // List<String>
    // Map<List<?>, List<>>
    // Map<?, List<>>
    // Map<List<String>, List<String>>
    // Map<Map<List<? extends Iterable>, List<>>, Map<List<>, List<>>>
    
    // cracking rules (version YZH):
    // <: identifier, '<', (identifier | '?' | '>') 
    // >: (identifier | '?' | '<' | '>'), '>', (',' | '>')  (depth>0)
    // ,: ('>' | '?' | identifier), ',', (identifier | '?')
    // ?: (',' | '<'), '?', (extends | super | ',' | '>')
    // identifier: (extends | super | ',' | '<'), identifier, ('<' | '>' | ',' | extends | super )
    
    int depth = 0; // mimics stack
    while (current) {
        //printCurrentToken(current);
        if(isSymbol('<', current)){
            depth++;
            if(isIdentifier(current->prev) && (isIdentifier(current->next) || isSymbol('?', current->next) || isSymbol('>', current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid '<' detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        if(isSymbol('>', current)){
            depth--;
            if(depth==0){
                return true;
            }
            // depth > 0
            if( (isIdentifier(current->prev) || isSymbol('?', current->prev) || isSymbol('<', current->prev) || isSymbol('>', current->prev)) && (isSymbol(',', current->next) || isSymbol('>', current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid '>' detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        if(isSymbol(',', current)){
            if( (isIdentifier(current->prev) || isSymbol('?', current->prev) || isSymbol('>', current->prev)) && (isSymbol('?', current->next) || isIdentifier(current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid ',' detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        if(isSymbol('?', current)){
            if( (isSymbol('<', current->prev) || isSymbol(',', current->prev)) ){
                if(isSymbol('?', current->next) || isSymbol('>', current->next)){
                    current = current->next;
                    continue;
                }
                if(isKey(EXTENDS, current->next) || isKey(SUPER, current->next)){
                    current = current->next->next;
                    continue;
                }
            }
            //printf("Debug: invalid '?' detected. Its previous and next nodes are:\n");
            //printCurrentToken(current->prev);
            //printCurrentToken(current->next);
            return false;
        }
        if(isIdentifier(current)){
            if( (isKey(EXTENDS, current->prev) || isKey(SUPER, current->prev) || isSymbol(',', current->prev) || isSymbol('<', current->prev)) && (isSymbol(',', current->next) || isSymbol('<', current->next) || isSymbol('>', current->next) || isKey(EXTENDS, current->next) || isKey(SUPER, current->next)) ){
                current = current->next;
                continue;
            }else{
                //printf("Debug: invalid identifier detected. Its previous and next nodes are:\n");
                //printCurrentToken(current->prev);
                //printCurrentToken(current->next);
                return false;
            }
        }
        //printf("Debug: invalid unknown token detected.\n");
        break; // invalid token
    }
    
    return false;
}

bool isPotentialAssignment(tokenNode* current){
    // <assignment> ::= <lvalue> <assignmentOperator> <expression>
    while(current && !isSemicolon(current) && !isSymbol(',', current)){
        if(isOperator("+=", current) || isOperator("-=", current) || isOperator("*=", current) || isOperator("/=", current)){
            return true;
        }
        if((isSymbol('=', current))){
            if(!isSymbol('<', current->prev) && !isSymbol('>', current->prev)){
                return true;
            }
        }
        current = current->next; 
    }
    
    return false;
}

bool isVariableDeclarationStart(tokenNode* current){
    // <variableDeclaration> ::=  [<accessModifier>] {<nonAccessModifier>} <type> [ '[' ']' ] ( <identifier> | <assignment>) {',' ( <identifier> | <assignment>)} 
    bool flag = false; // use this flag to record whether there is a type
    if(isKey(PUBLIC, current) || isKey(PRIVATE, current)){
        current = current->next;
    }
    
    while(isKey(STATIC, current) || isKey(FINAL, current)){
        current = current->next;
    }
    //printf("Debug: comsumed modifiers. next:\n");
    //printCurrentToken(current);
    if(isKey(CHAR, current) || isKey(INT, current) || isKey(LONG, current) || isKey(DOUBLE, current) || isKey(BOOLEAN, current)){
        current = current->next;
        flag = true;
        printf("Debug: flag set to true -- a keyword\n");
    }else if(isIdentifier(current)){
        current = current->next;
        flag = true;
        //printf("Debug: comsumed identifier. next:\n");
        //printCurrentToken(current);
        printf("Debug: flag set to true -- an identifier\n");
        if(isSymbol('<', current)){
            current = current->next;
            int depth = 1;
            while(depth>0){
                if(isSymbol('<', current)){
                    depth++;
                }else if(isSymbol('>', current)){
                    depth--;
                }else if(isSemicolon(current)){
                    return false;
                }else if(!current){
                    return false;
                }
                current = current->next;
            }
        }
        
    }else{
        return false;
    }
    
    while(isBracket('[', current)){
        current = current->next;
        if(isBracket(']', current)){
            current = current->next;
        }else{
            return false;
        }
    }
    
    //printf("Debug: 0reached here\n");
    //printCurrentToken(current);
    if(isIdentifier(current)){
        current = current->next;
    }else{
        return false;
    }
    //printf("Debug: 1reached here\n");
    
    if(!isBracket('(', current)){
        return flag;
    }
    
    return false;
}

bool isPotentialSubroutineCall(tokenNode* current){
    // a subroutine call must be like:
    // <id> [ { '.' <id> [ '(' <expressionList> ')' ] } '.' <id> ] '(' <expressionList> ')'
    // key: an identifier is followed either by a '(' or a '.'.
    // once the above condition is not satisfied, probe the last token and 
    // return last token == ')'.
    
    // the subroutine call must start with an identifier representing the object.
    
    if(isIdentifier(current)){
        current = current->next;
    }else{
        return false;
    }
    
    // if a '.' follows, its a field access or method call. if a '(' follows, we should conclude the subroutine call soon.
    if(isBracket('(', current)){
        current = current->next;
        int depth = 1;
        while(depth>0){
            if(isBracket('(', current)){
                depth++;
            }else if(isBracket(')', current)){
                depth--;
            }else if(isSemicolon(current)){
                return false;
            }else if(!current){
                return false;
            }
            if(depth>0){
                current = current->next;
            }
        }
        return true;
    }else if(isSymbol('.', current)){
        // skip, we leave the dot to be handled in the loop
    }else{
        return false;
    }
    
    while(isSymbol('.', current)){
        current = current->next;
        // it must be an identifier after '.'
        if(isIdentifier(current)){
            current = current->next;
        }else{
            return false;
        }
        // after the identifier it's an optional parenthesis
        if(isBracket('(', current)){
            current = current->next;
            int depth = 1;
            while(depth>0){
                if(isBracket('(', current)){
                    depth++;
                }else if(isBracket(')', current)){
                    depth--;
                }else if(isSemicolon(current)){
                    return false;
                }else if(!current){
                    return false;
                }
                current = current->next;
            }
        }
    }
    
    // final step: query current->prev
    return isBracket(')', current->prev);
} 

bool isStatementStart(tokenNode* current){
    if(isSemicolon(current) || isBracket('{', current)){
        return true;
    }
    if(isKey(IF, current) || isKey(SWITCH, current) || isKey(FOR, current) || isKey(WHILE, current) || isKey(DO, current) || isKey(RETURN, current) || isKey(BREAK, current) || isKey(CONTINUE, current) || isKey(STATIC, current)){
        return true;
    }
    if(isVariableDeclarationStart(current)){
        return true;
    }
    if(isExpressionStart(current)){
        return true;
    }
    if(isPotentialAssignment(current)){
        return true;
    }
    return false;
}

bool isClassStart(tokenNode* current){
    while(isKey(PUBLIC, current) || isKey(PRIVATE, current) || isKey(ABSTRACT, current) || isKey(STATIC, current) || isKey(FINAL, current)){
        current = current->next;
    }
    
    if(isKey(CLASS, current)){
        return true;
    }
    return false;
}

bool isInterfaceStart(tokenNode* current){
    while(isKey(PUBLIC, current) || isKey(PRIVATE, current)){
        current = current->next;
    }
    
    if(isKey(INTERFACE, current)){
        return true;
    }
    return false;
}

bool isPotentialType(tokenNode* current){
    if(isKey(CHAR, current) || isKey(INT, current) || isKey(LONG, current) || isKey(BOOLEAN, current) || isKey(DOUBLE, current)){
        return true;
    }
    
    if(isIdentifier(current)){
        current = current->next;
    }
    
    if(isSymbol('<', current)){
        return isPotentialGenerics(current);
    }else{
        return true;
    }
}

bool isSubroutineDeclarationStart(tokenNode* current){
    if(isSymbol('@', current)){
        current = current->next;
        if(isIdentifier(current)){
            current = current->next;
        }else{
            return false;
        }
    }

    while(isKey(PUBLIC, current) || isKey(PRIVATE, current) || isKey(STATIC, current) || isKey(FINAL, current) || isKey(ABSTRACT, current) || isKey(NATIVE, current)){
        current = current->next;
    }
    
    if(isKey(VOID, current)){
        return true;
    }
    
    if(isSymbol('<', current)){
        int depth = 1;
        current = current->next;
        while(depth>0){
            if(isSymbol('<', current)){
                depth++;
            }else if(isSymbol('>', current)){
                depth--;
            }
            current = current->next;
        }
    }
    
    return isPotentialType(current);
}

bool isPotentialCasting(tokenNode* current){
    if(isBracket('(', current)){
        current = current->next;
    }else{
        return false;
    }
    
    if(isKey(CHAR, current) || isKey(INT, current) || isKey(LONG, current) || isKey(BOOLEAN, current) || isKey(DOUBLE, current)){
        current = current->next;
    }else if(isIdentifier(current)){
        current = current->next;
    }else{
        return false;
    }
    
    if(isSymbol('<', current)){
        if(isPotentialGenerics(current)){
            int depth = 1;
            current = current->next;
            while(depth>0){
                if(isSymbol('<', current)){
                    depth++;
                }else if(isSymbol('>', current)){
                    depth--;
                }
                current = current->next;
            }
            if(isBracket(')', current)){
                return true;
            }else{
                return false;
            }
        }else{
            return false;
        }
    }else if(isBracket(')', current)){
        return true;
    }
    
    return false;
    
}

void checkKeyValueNodeExpected(tokenNode* n, tokenType expectedType, keyword expectedValue, char* functionName, char* errorMessage){
    if(!n || n->t->type != expectedType || n->t->data.key_val != expectedValue){
        fprintf(stderr, "%sError %s line %d: %s.%s\n", RED, functionName, n->t->lineNumber, errorMessage, NRM);
        exit(1);
    }
}

void checkCharValueNodeExpected(tokenNode* n, tokenType expectedType, char expectedValue, char* functionName, char* errorMessage){
    if(expectedValue == -1){
        if(!n || n->t->type != expectedType){
            fprintf(stderr, "%sError %s line %d: %s.%s\n", RED, functionName, n->t->lineNumber, errorMessage, NRM);
            exit(1);
        }
    }
    else{
        if(!n || n->t->type != expectedType || n->t->data.char_val != expectedValue){
            fprintf(stderr, "%sError %s line %d: %s.%s\n", RED, functionName, n->t->lineNumber, errorMessage, NRM);
            exit(1);
        }
    }
}

void checkStringValueNodeExpected(tokenNode* n, tokenType expectedType, char* expectedValue, char* functionName, char* errorMessage){
    if(expectedValue == NULL){
        if(!n || n->t->type != expectedType){
            fprintf(stderr, "%sError %s line %d: %s.%s\n", RED, functionName, n->t->lineNumber, errorMessage, NRM);
            exit(1);
        }
    }
    else{
        if(!n || n->t->type != expectedType || strcmp(n->t->data.str_val, expectedValue)){
            fprintf(stderr, "%sError %s line %d: %s.%s\n", RED, functionName, n->t->lineNumber, errorMessage, NRM);
            exit(1);
        }
    }
}

treeNode* insertNewNode2Parent(ruletype rule, token* t, treeNode* parent){
    treeNode* child = createTreeNode(rule, t);
    child->parent = parent;
    insertChildNode(parent, child);
    return child;
}

treeNode* createTreeNode(ruletype rule, token* t){
    treeNode* n = calloc(1,sizeof(treeNode));
    if(!n){
        fprintf(stderr, "%sError createTreeNode line %d: not enough memory, cannot create tree node.%s\n", RED, t->lineNumber, NRM);
        exit(1);
    } 
    n->ruleType = rule;
    n->assoToken = t;
    n->children = calloc(16, sizeof(treeNode*));
    if(!n->children){
        fprintf(stderr, "%sError createTreeNode line %d: not enough memory, cannot create children array.%s\n", RED, t->lineNumber, NRM);
        exit(1);
    } 
    n->capacity = 16;
    return n;
}

void insertChildNode(treeNode* n, treeNode* child){
    if(!n || !child){
        fprintf(stderr, "%sError insertChildNode: null pointer provided.%s\n", RED, NRM);
        exit(1);
    }
    if(n->capacity < n->childCount){
        if(n->assoToken){
            fprintf(stderr, "%sError insertChildNode line %d: code of parser has destructively wrong logic.%s\n", RED, n->assoToken->lineNumber, NRM);
        }else{
            fprintf(stderr, "%sError insertChildNode: code of parser has destructively wrong logic.%s\n", RED, NRM);
        }
        exit(1);
    }
    if(n->capacity == n->childCount){
        n->children = (treeNode**)realloc(n->children, 2*(n->capacity)*sizeof(treeNode*));
        if(!n->children){
            if(n->assoToken){
                fprintf(stderr, "%sError insertChildNode line %d: not enough memory, cannot realloc children array.%s\n", RED, n->assoToken->lineNumber, NRM);
            }else{
                fprintf(stderr, "%sError insertChildNode: not enough memory, cannot realloc children array.%s\n", RED, NRM);
            }
            exit(1);
        }
        n->capacity = 2 * (n->capacity);
    }
    n->children[n->childCount++] = child;
    child->parent = n;
}

void printCST(CST* cst){
    if(!cst){
        return;
    }
    printTreeNode(cst->root, 0);
}

void printTreeNode(treeNode* n, int indent){
    if(!n){
        return;
    }
    for(int i=0; i<indent; i++){
        printf(" ");
    }
    if(!n->assoToken){
        printf("Rule = %s", getRule(n->ruleType));
    }
    else{
        switch(n->assoToken->type){
            case KEYWORD:
                printf("TokenType = Keyword, TokenValue = %s, lineNumber = %d", getKeyword(n->assoToken->data.key_val), n->assoToken->lineNumber);
                break;
            case NUMBER:
                printf("TokenType = Number, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case CHARACTER:
                printf("TokenType = Character, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case IDENTIFIER:
                printf("TokenType = Identifier, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case OPERATOR:
                printf("TokenType = Operator, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case STRING:
                printf("TokenType = String, TokenValue = %s, lineNumber = %d", n->assoToken->data.str_val, n->assoToken->lineNumber);
                break;
            case SYMBOL:
                printf("TokenType = Symbol, TokenValue = %c, lineNumber = %d", n->assoToken->data.char_val, n->assoToken->lineNumber);
                break;
            case BRACKET:
                printf("TokenType = Bracket, TokenValue = %c, lineNumber = %d", n->assoToken->data.char_val, n->assoToken->lineNumber);
                break;
            case SEMICOLON:
                printf("TokenType = Semicolon, TokenValue = %c, lineNumber = %d", n->assoToken->data.char_val, n->assoToken->lineNumber);
                break;
        }
    }
    if(n->parent){
        printf(", ParentRule = %s", getRule(n->parent->ruleType));
    }
    if(n->childCount){
        printf(", ChildRules: ");
        for(size_t i=0; i<n->childCount; i++){
            printf("%s, ", getRule(n->children[i]->ruleType));
        }
    }
    printf(".\n");
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            printTreeNode(n->children[i], indent+2); 
        }
        
    }
}

void printLessCST(CST* cst){
    if(!cst){
        return;
    }
    printLessTreeNode(cst->root, 0);
}

void printLessTreeNode(treeNode* n, int indent){
    bool flag = false;
    if(!n){
        return;
    }
    if(indent==0 || n->assoToken || n->childCount>1){
        flag = true;
    }
    
    if(flag){
    for(int i=0; i<indent; i++){
        printf(" ");
    }
    if(!n->assoToken){
        printf("Rule = %s, ", getRule(n->ruleType));
    }
    else{
        printf("Rule = %s, ", getRule(n->ruleType));
        switch(n->assoToken->type){
            case KEYWORD:
                printf("TokenValue = %s%s%s, ", GRN, getKeyword(n->assoToken->data.key_val), NRM);
                break;
            case NUMBER:
                printf("TokenValue = %s%s%s, ", GRN, n->assoToken->data.str_val, NRM);
                break;
            case CHARACTER:
                printf("TokenValue = %s%s%s, ", GRN, n->assoToken->data.str_val, NRM);
                break;
            case IDENTIFIER:
                printf("TokenValue = %s%s%s, ", GRN, n->assoToken->data.str_val, NRM);
                break;
            case OPERATOR:
                printf("TokenValue = %s%s%s, ", GRN, n->assoToken->data.str_val, NRM);
                break;
            case STRING:
                printf("TokenValue = %s%s%s, ", GRN, n->assoToken->data.str_val, NRM);
                break;
            case SYMBOL:
                printf("TokenValue = %s%c%s, ", GRN, n->assoToken->data.char_val, NRM);
                break;
            case BRACKET:
                printf("TokenValue = %s%c%s, ", GRN, n->assoToken->data.char_val, NRM);
                break;
            case SEMICOLON:
                printf("TokenValue = %s%c%s, ", GRN, n->assoToken->data.char_val, NRM);
                break;
        }
    }
    
    if(n->parent){
        printf("ParentRule = %s, ", getRule(n->parent->ruleType));
    }
    if(n->childCount){
        printf("ChildRules: ");
        for(size_t i=0; i<n->childCount; i++){
            printf("%s%s%s, ", GRN, getRule(n->children[i]->ruleType), NRM);
        }
    }
    printf("%s\n", NRM);
    }
    if(n->childCount){
        for(size_t i=0; i<n->childCount; i++){
            printLessTreeNode(n->children[i], indent+(flag?2:0)); 
        }
        
    }
}

char* getRule(ruletype r){
    static char* rules[] = {
    "identifier",
    "bracket",
    "compound",
    "operator",
    "semicolon",
    "type",
    "primitiveType",
    "referenceType",
    "generics",
    "typeArgument",
    "wildcard",
    "langle",
    "rangle",
    "comma",
    "extends",
    "super",
    "implements",
    "annotation",
    "at",
    "term",
    "parenthesizedExpression",
    "newObject",
    "new",
    "arrayInitialization",
    "terminalTerm",
    "number",
    "character",
    "string",
    "dot",
    "fieldAccess",
    "arrayAccess",
    "subroutineCall",
    "expressionList",
    "expression",
    "ternaryExpression",
    "ternaryOperator",
    "logicalOrExpression",
    "logicalAndExpression",
    "bitwiseOrExpression",
    "bitwiseXorExpression",
    "bitwiseAndExpression",
    "equalityExpression",
    "relationalExpression",
    "instanceof",
    "shiftExpression",
    "additiveExpression",
    "multiplicativeExpression",
    "castExpression",
    "unaryExpression",
    "postfixExpression",
    "assignment",
    "assignmentOperator",
    "variableDeclaration",
    "accessModifier",
    "nonAccessModifier",
    "subroutineDeclaration",
    "native",
    "parameterList",
    "const",
    "typeBoundList",
    "typeBound",
    "constraint",
    "moreInterface",
    "subroutineBody",
    "statement",
    "ifStatement",
    "if",
    "else",
    "switchStatement",
    "switch",
    "case",
    "colon",
    "default",
    "forStatement",
    "for",
    "whileStatement",
    "while",
    "doWhileStatement",
    "do",
    "returnStatement",
    "return",
    "continueStatement",
    "continue",
    "breakStatement",
    "break",
    "staticStatement",
    "static",
    "codeBlock",
    "classDeclaration",
    "abstract",
    "class",
    "interfaceDeclaration",
    "interface",
    "classBody",
    "interfaceBody",
    "importStatement",
    "import",
    "file"
        };
    return rules[r];
}

void freeCST(CST** cst){
    if(!cst || !(*cst)){
        return;
    }
    freeTreeNode((*cst)->root);
    free(*cst);
    *cst = NULL;
}

void freeTreeNode(treeNode* n){
    if(!n){
        return;
    }
    for(size_t i=0; i < n->childCount; i++){
        freeTreeNode(n->children[i]);
    }
    free(n->children);
    free(n);
}
