#include <stdio.h>
#include <stdlib.h>

#include "error.h"

void sairErro(FILE* file, int codigoErro, char* tipoErro) {
    fclose(file);
    printf(tipoErro);
    
    exit(codigo);
}