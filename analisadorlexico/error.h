#ifndef error
#define error

extern int linha;
extern int coluna;

typedef enum {
    operacaoSucesso,
    arquivoNaoPassado,
    arquivoNaoCarregado,
    
    excedeuMaxLengthLexico,
    tokenInexperado,
} CodigoErro;

void sairErro(FILE* file, int codigoErro, char* tipoErro);
void printCurrentFilePosition(FILE* file);

#endif