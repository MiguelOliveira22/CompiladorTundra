#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "basics.h"
#include "error.h"
#include "lista.h"
#include "tokenlexico.h"

#include "memory.h"

static Lista* identifiersToClose;
static Lista* numbersToClose;

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

    inserirElementoLista(identifiersToClose, identificador);
}

void closeIdentifiers() {
    if (identifiersToClose == NULL) {
        return;
    }

    destruirLista(identifiersToClose);
}