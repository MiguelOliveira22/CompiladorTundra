#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "memory.h"

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
