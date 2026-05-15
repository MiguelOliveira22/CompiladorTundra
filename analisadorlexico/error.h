#ifndef error
#define error

typedef enum {
    arquivoNaoReconhecido,
    arquivoNaoCarregado,
    
} CodigoErro;

void sairErro(FILE* file, int codigoErro, char* tipoErro);

#endif