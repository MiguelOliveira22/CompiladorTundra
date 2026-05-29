#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

extern int linha;
extern int coluna;

void printCurrentFilePosition(FILE* file);

void sairErro(FILE* file, int codigoErro, char* tipoErro) {
    printCurrentFilePosition(file);
    
    fclose(file);
    printf(tipoErro);
    
    exit(codigoErro);
}

void printCurrentFilePosition(FILE* file) {
    
    printf("(%d, %d)\n\n", linha, coluna);
}