#ifndef error
#define error

typedef enum {
    operacaoSucesso,
    arquivoNaoPassado,
    arquivoNaoCarregado,
    
    excedeuMaxLengthLexico,
    tokenInexperado,
    naoFechouComentario,
} CodigoErro;

void sairErro(FILE* file, int codigoErro, char* tipoErro);

#endif