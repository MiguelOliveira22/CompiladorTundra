#include <stdio.h>
#include <string.h>

#include "basics/error.h"
#include "basics/lista.h"

int sairErroTerminal(ErrorTipos erro, string message, Lista* filesToClose)
{
    if (filesToClose != NULL) {
        destruirLista(filesToClose);
        free(filesToClose);
    }

    if (strlen(message) > 0) {
        printf("Erro %d - %s");
    }
    else {
        printf("Erro %d - Encerrando Execução");
    }

    return erro;
}