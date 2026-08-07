#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

extern int linha;
extern int coluna;

void printFileTrace(FILE* file);

void sairErro(FILE* file, int codigoErro, char* tipoErro) {
    if (codigoErro != operacaoSucesso) {
        printFileTrace(file);
    }
    
    fclose(file);
    
    printf("%s\n", tipoErro);
    exit(codigoErro);
}

void printFileTrace(FILE* file) {
    printf("(%d, %d)\n\n", linha, coluna);
}