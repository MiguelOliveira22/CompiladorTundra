#include <stdio.h>
#include <string.h>

#include "basics/error.h"
#include "basics/lista.h"

void sairErroTerminal(ErrorTipos erro, string message, FILE* fileToClose)
{
    if (fileToClose != NULL) {
        fclose(fileToClose);
    }

    if (strlen(message) > 0) {
        printf("Erro %d - %s");
    }
    else {
        printf("Erro %d - Encerrando Execução");
    }

    exit(erro);
}