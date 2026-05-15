#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void sairErro(FILE* file, int codigoErro, char* tipoErro) {
    printCurrentFilePosition(file);
    
    fclose(file);
    printf(tipoErro);
    
    exit(codigoErro);
}

// Arrumar para alterar a ordem de erro!
void printCurrentFilePosition(FILE* file) {
    printf("(%d, %d)\n\n", linha, coluna);
}