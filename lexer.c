#include "lexer.h"

static int lineNumber = 1;

tokenTable* lexFile(FILE* f){
    if(!f){
        return NULL;
    }
    tokenTable* ttable = createTokenTable();
    if(!ttable){
        fprintf(stderr, "Error lexFile: not enough heap memory, cannot create token table.\n");
        exit(1);
    }
    
    hll* h = NULL;
    
    char prev;
    char c;
    
    // if we encounter a \", turn in_string on until we reach a \" without a previous attached \\.
    bool in_string = false;
    // if we encounter a digit, turn in_number on until we reach anything that is not a digit or . . if we encounter ., lookahead its next char, if it's not a digit, treat the current dot as symbol.
    bool in_number = false;
    // if we encounter a letter or underscore, turn in_id on. turn it off when reach anything else than letter, digit or underscore.
    bool in_id = false;
    // if we encounter a // ,turn it on. turn it off when reach a newline .
    bool in_one_comment = false;
    // /* */
    bool in_multi_comment = false;
    
    while((c=fgetc(f))!=EOF){
        if(c=='\n'){
            printf("Debug: next token = newline\n");
        }else{
            printf("Debug: next token = %c\n", c);
        }
        // at most one of the flags can be on.
        // if flags are all off:
        if(!in_string && !in_number && !in_id && !in_one_comment && !in_multi_comment){
            // we encounter a newline, should increment line number.
            // if you are a Mac user, good luck.
            if(c=='\n'){
                lineNumber++;
            }
            // we encounter a white space, skip current char.
            else if(isspace(c)){
                continue;
            }
            // we encounter a '/', first check if it has a next '/' or '*'. if no, throws error.
            else if(c=='/'){
                char next = fgetc(f);
                if(next == '/'){
                    in_one_comment = true;
                }else if (next == '*'){
                    in_multi_comment = true;
                    /*
                    dont know why the newline after multi comment is skipped, but with a continue fixed this. poo. 
                    */
                    continue; 
                }else{
                    char tmp[2]={0};
                    tmp[0]=c;
                    //printf("Debug: symbol %c inserted into table.\n",c);
                    writeToken(lexToken(tmp, lineNumber), ttable);
                }
                ungetc(next, f);
            }
            // we encounter a \", turn on in_string.
            else if(c=='\"'){
                h = createHLL();
                if(!h){
                    fprintf(stderr, "Error lexFile line %d: not enough heap memory, cannot create hybrid linked list.\n", lineNumber);
                    exit(1);
                }
                insert2HLL(c, h);
                //printf("Debug: insert %c into buffer.\n",c);
                in_string = true;
                prev = '\"';
            }
            // we encounter a letter or underscore, turn on in_id.
            else if(isalpha(c) || c=='_'){
                h = createHLL();
                if(!h){
                    fprintf(stderr, "Error lexFile line %d: not enough heap memory, cannot create hybrid linked list.\n", lineNumber);
                    exit(1);
                }
                insert2HLL(c, h);
                //printf("Debug: 1insert %c into buffer.\n",c);
                in_id = true;
            }
            // we encounter a digit, turn on in_number.
            else if(isdigit(c)){
                h = createHLL();
                if(!h){
                    fprintf(stderr, "Error lexFile line %d: not enough heap memory, cannot create hybrid linked list.\n", lineNumber);
                    exit(1);
                }
                insert2HLL(c, h);
                //printf("Debug: insert %c into buffer.\n",c);
                in_number = true;
            }
            // we encounter a . , should lookahead its next char to determine.
            else if(c=='.') {
                char next = fgetc(f);
                if(isdigit(next)){
                    h = createHLL();
                    if(!h){
                        fprintf(stderr, "Error lexFile line %d: not enough heap memory, cannot create hybrid linked list.\n", lineNumber);
                        exit(1);
                }
                insert2HLL(c, h);
                //printf("Debug: insert %c into buffer.\n",c);
                in_number = true;
                
                }else{
                    char tmp[2]={0};
                    tmp[0]=c;
                    //printf("Debug: symbol %c inserted into table.\n",c);
                    writeToken(lexToken(tmp, lineNumber), ttable);
                }
                ungetc(next, f);
            }
            // else, curent char might be a symbol, bracket, semicolon or unknown char.
            else{
                char tmp[2]={0};
                tmp[0]=c;
                //printf("Debug: symbol %c inserted into table.\n",c);
                writeToken(lexToken(tmp, lineNumber), ttable);
            }
        }
        
        else if(in_one_comment){
            if(c=='\n'){
                in_one_comment = false;
                lineNumber++;
            }
        }
        else if(in_multi_comment){
            if(c=='\n'){
                lineNumber++;
            }else if(c=='*'){
                char next = fgetc(f);
                if(next == '/'){
                    in_multi_comment = false;
                }
            }
        }
        
        // if in_string is on, we exit at a \" without previous \\.
        else if(in_string){
            // expected exit
            if(prev != '\\' && c == '\"'){
                in_string = false;
                insert2HLL(c, h);
                //printf("Debug: insert %c into buffer.\n",c);
                prev = c;
                char* str = hll2str(h);
                //printf("Debug: string is %s\n",str);
                writeToken(lexToken(str, lineNumber), ttable);
                free(str);
                freeHLL(&h);
            }
            // unexpected exit
            else if(c == EOF){
                fprintf(stderr, "Error lexFile line %d: unfinished string literal.\n", lineNumber);
                exit(1);
            }
            // non printable char
            else if(!isprint(c)){
                fprintf(stderr, "Error lexFile line %d: non-printable char detected in string literal.\n", lineNumber);
                exit(1);
            }
            else{
                insert2HLL(c, h);
                //printf("Debug: insert %c into buffer.\n",c);
                prev = c;
            }
        }
        
        // if in_number is on, we exit at anything that's not a digit or dot.
        else if(in_number){
            // expected exit
            if(c != '.' && !isdigit(c)){
                in_number = false;
                char* str = hll2str(h);
                //printf("Debug: number is %s\n",str);
                writeToken(lexToken(str, lineNumber), ttable);
                free(str);
                freeHLL(&h);
                ungetc(c, f);
            }
            else{
                insert2HLL(c, h);
                //printf("Debug: insert %c into buffer.\n",c);
            }
        }
        
        // if in_id is on, we exit at anything that's not a digit, letter or underscore.
        else if(in_id){
            // expected exit
            if(c != '_' && !isalnum(c)){
                in_id = false;
                char* str = hll2str(h);
                //printf("Debug: identifier is %s\n",str);
                writeToken(lexToken(str, lineNumber), ttable);
                free(str);
                freeHLL(&h);
                ungetc(c, f);
            }
            else{
                insert2HLL(c, h);
                //printf("Debug: 2insert %c into buffer.\n",c);
            }
        }
    }
    freeHLL(&h);
    return ttable;
}

token* lexToken(char* str, int lineNumber){
    if(!str || !str[0]){
        return NULL;
    }
    
    token* t = createToken();
    if(!t){
        fprintf(stderr, "Error lexToken line %d: not enough heap memory, cannot create token.\n", lineNumber);
        exit(1);
    }
    t->lineNumber = lineNumber;
    int len = strlen(str);
    
    if(len == 1){
        if(str[0]=='+' || str[0]=='-' || str[0]=='*' || str[0]=='/' || str[0]=='%' || str[0]=='?' || str[0]==':' || str[0]=='|' || str[0]=='&' || str[0]=='!' || str[0]=='~' || str[0]=='^' || str[0]=='>' || str[0]=='<' || str[0]=='=' || str[0]=='@' || str[0]=='.' || str[0]=='\'' || str[0]==','){
            t->type = SYMBOL;
            t->data.char_val = str[0];
            return t;
        }
        if(str[0]=='{' || str[0]=='}' || str[0]=='(' || str[0]==')' || str[0]=='[' || str[0]==']'){
            t->type = BRACKET;
            t->data.char_val = str[0];
            return t;
        }
        if(str[0]==';'){
            t->type = SEMICOLON;
            t->data.char_val = ';';
            return t;
        }
    }
    
    int KEY = isKeyword(str);
    if(KEY>=0){
            t->type = KEYWORD;
            t->data.key_val = KEY;
            return t;
    }
    
    
    if((len>=1 && isdigit(str[0])) || (len>1 && (str[0]=='-' || str[0]=='.'))){
        // this might be a number, need further verification:
        int countdot=0;
        int countminus=0;
        bool isnumber=true;
        for(int i=0;i<len;i++){
            if(str[i]=='.'){
                countdot++;
            }
            if(str[i]=='-'){
                countminus++;
            }
            if(!isdigit(str[i]) && str[i]!='.' && str[i]!='-'){
                isnumber = false;
            }
            if(countdot>1 || countminus>1){
                isnumber = false;
            }
        }
        if(isnumber){
            t->type = NUMBER;
            t->data.str_val = calloc(len+1, sizeof(char));
            strcpy(t->data.str_val, str);
            return t;
        }
    }
    
    if(str[0]=='\"' && str[len-1]=='\"'){
        t->type = STRING;
        t->data.str_val = calloc(len+1, sizeof(char));
        strcpy(t->data.str_val, str);
        return t;
    }
    
    // an identifier cannot start with a digit.
    if(isdigit(str[0])){
        fprintf(stderr, "Error lexToken line %d: identifier %s cannnot start with a digit.\n", lineNumber, str);
        exit(1);
    }
    
    bool isid=true;
    for(int i=0;i<len;i++){
        if(!isalnum(str[i]) && str[i]!='_'){
            isid = false;
        }
    }
    if(isid){
        t->type = IDENTIFIER;
        t->data.str_val = calloc(len+1, sizeof(char));
        strcpy(t->data.str_val, str);
        return t;
    }
    
    
    
    // deal with unknown token.
    fprintf(stderr, "Error lexToken line %d: unknown identifier %s.\n", lineNumber, str);
    exit(1);
}

hll* createHLL(){
    return (hll*)calloc(1,sizeof(hll));
}

void insert2HLL(char c, hll* h){
    if(!h){
        fprintf(stderr, "Error insert2HLL line %d: inserting into null buffer.\n", lineNumber);
        exit(1);
    }
    bool flag = true;
    while(flag){
        if(h->current>=256){
            if(!h->next){
                hll* h2 = createHLL();
                if(!h2){
                    fprintf(stderr, "Error insert2HLL line %d: not enough heap memory, cannot create hybrid linked list.\n", lineNumber);
                    exit(1);
                }
                h->next = h2;
                h = h2;
                flag = false;
            }else{
                h = h->next;
            }
        }else{
            flag = false;
        }
    }
    
    h->buffer[h->current] = c;
    (h->current)++;
}

char* hll2str(hll* h){
    if(!h){
        return NULL;
    }
    int len = 0;
    hll* p = h;
    while(p){
        len += p->current;
        p = p->next;
    }
    char* str = calloc(len + 1, sizeof(char));
    if(!str){
        fprintf(stderr, "Error hll2str line %d: not enough heap memory, cannot create string.\n", lineNumber);
        exit(1);
    }
    int n = 0;
    while(h){
        for(int i = 0;i < h->current;i++){
            str[n+i] = h->buffer[i];
        }
        n = n + h->current;
        h = h->next;
    }
    return str;
}

void freeHLL(hll** h){
    if(!h){
        return;
    }
    hll* p = *h;
    while(p){
        hll* tmp = p->next;
        free(p);
        p = tmp;
    }
    *h = NULL;
}
