#ifndef analisadorlexico

#define analisadorlexico

typedef unsigned char i8;

typedef enum {
    false, true
} bool;

typedef struct {
    char* lexico;
    bool  isBreak;
    char* tokenValor;
} token;

void compilaPrograma(FILE* file, int currentScope);
void compilaBloco(FILE* file, int currentScope);
void sairErro(int codigo, FILE* file);

token  analisarArquivo(FILE* file);
token  getToken(char* palavra);
bool   isIdentifier(char* word);
bool   isNumeric(char* word);
int    getNumeric(char* word);

#endif