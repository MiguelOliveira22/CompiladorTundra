#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "basics.h"
#include "error.h"
#include "lista.h"
#include "tokenlexico.h"
#include "memory.h"

static Lista* stringsToClose;
static Lista* identifiersToClose;
static Lista* numbersToClose;
static Lista* ponteirosToClose;

int compareIdentifiers(Token* identifierA, Token* identifierB) {
    return strcmp(identifierA->identificador, identifierB->identificador);
}

void mostrarIdentifier(Token* identifier) {
    printf("[ID] %36s", identifier->identificador);
}

void liberarIdentifier(Token* identifier) {
    free(identifier->identificador);
    free(identifier);
}

void cadastrarIdentifiersToClose(Token* identificador) {
    if (identifiersToClose == NULL) {
        identifiersToClose = (Lista*) malloc(sizeof(Lista));

        if (identifiersToClose == NULL) {
            sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
        }

        construirLista(identifiersToClose, compareIdentifiers, mostrarIdentifier, liberarIdentifier);
    }

    inserirElementoEmOrdemLista(identifiersToClose, identificador);
}

void closeIdentifiers() {
    if (identifiersToClose == NULL) {
        return;
    }

    destruirLista(identifiersToClose);
}

void closePonteiros() {
    if (ponteirosToClose == NULL) {
        return;
    }

    destruirLista(ponteirosToClose);
}

void associarPonteirosParaErros(Elemento ponteiro, void (*d) (Elemento a)) {
    if (ponteirosToClose == NULL) {
        ponteirosToClose = (Lista*) malloc(sizeof(Lista));
        construirLista(ponteirosToClose, NULL, NULL, NULL);
    }

    PonteiroDestrutivel* novoDado = (PonteiroDestrutivel*) malloc(sizeof(PonteiroDestrutivel));
    novoDado->dados = ponteiro;
    novoDado->d = d;

    inserirElementoNoFinalLista(ponteirosToClose, novoDado);
}