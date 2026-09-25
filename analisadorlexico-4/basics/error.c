#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "lista.h"

static Lista* ponteirosParaDestruir = NULL;

static void destruirPonteiros() {
    if (ponteirosParaDestruir == NULL) {
        return;
    }

    destruirLista(ponteirosParaDestruir);
}

void associarPonteirosParaErros(Elemento* ponteiro) {
    if (ponteirosParaDestruir == NULL) {
        construirLista(ponteirosParaDestruir, );
    }
}

void sairErroTerminal(ErrorTipos erro, string message)
{
    destruirPonteiros();

    if (strlen(message) > 0) {
        printf("Erro %d - %s", erro, message);
    }
    else {
        printf("Erro %d - Encerrando Execução", erro);
    }

    exit(erro);
}
