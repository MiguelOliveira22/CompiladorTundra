#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "memorymng.h"

void sairErroTerminal(ErrorTipos erro, string message)
{
    closePonteiros();

    if (erro == ERROR_OK_OPERATION_SUCCESS) {
        printf("OK Sucesso - %s\n", message);
    }
    else if (strlen(message) > 0) {
        printf("Erro %d - %s\n", erro, message);
    }
    else {
        printf("Erro %d - Encerrando Execução\n", erro);
    }

    exit(erro);
}

void printAvisoTerminal(AvisoTipos aviso, string message) {
    if (strlen(message) > 0) {
        printf("Aviso %d - %s\n", aviso, message);
    }
    else {
        printf("Aviso %d - Observação\n", aviso);
    }
}
