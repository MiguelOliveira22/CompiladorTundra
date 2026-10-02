#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "basics.h"
#include "error.h"
#include "lista.h"
#include "tokenlexico.h"

#include "memorymng.h"

static Lista* stringsToClose;
static Lista* identifiersToClose;
static Lista* numbersToClose;
static Lista* ponteirosToClose;

static int compareIdentifiers(Elemento identifierA, Elemento identifierB) {
    return strcmp(((Token*) identifierA)->identificador, ((Token*) identifierB)->identificador);
}

static void mostrarIdentifier(Elemento identifier) {
    printf("[ID] %36s", ((Token*) identifier)->identificador);
}

static void liberarIdentifier(Elemento identifier) {
    free(((Token*) identifier)->identificador);
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

    inserirElementoNoFinalLista(identifiersToClose, identificador);
}

void closeIdentifiers() {
    if (identifiersToClose == NULL) {
        return;
    }

    destruirLista(identifiersToClose);
}

static int compareStrings(Elemento stringA, Elemento stringB) {
    return strcmp((string) stringA, (string) stringB);
}

static void mostrarString(Elemento stringPrint) {
    printf("[STR] %s", (string) stringPrint);
}

static void freeString(Elemento stringFree) {
    free(stringFree);
}

void cadastrarStringsToClose(string content) {
    if (stringsToClose == NULL) {
        stringsToClose = (Lista*) malloc(sizeof(Lista));

        if (stringsToClose == NULL) {
            sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
        }

        construirLista(stringsToClose, compareStrings, mostrarString, freeString);
    }

    inserirElementoNoFinalLista(stringsToClose, content);
}

void closeStrings() {
    if (stringsToClose == NULL) {
        return;
    }

    destruirLista(stringsToClose);
}

void cadastrarPonteirosToClose(Elemento ponteiro, void (*d) (Elemento a)) {
    if (ponteirosToClose == NULL) {
        ponteirosToClose = (Lista*) malloc(sizeof(Lista));

        if (ponteirosToClose == NULL) {
            sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
        }

        construirLista(ponteirosToClose, NULL, NULL, NULL);
    }

    PonteiroDestrutivel* novoDado = (PonteiroDestrutivel*) malloc(sizeof(PonteiroDestrutivel));

    if (novoDado == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }

    novoDado->dados = ponteiro;
    novoDado->d = d;

    inserirElementoNoFinalLista(ponteirosToClose, novoDado);
}

void closePonteiros() {
    if (ponteirosToClose == NULL) {
        return;
    }

    NoLista* atual = ponteirosToClose->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL)
    {
        if (anterior != NULL) {
            if (((PonteiroDestrutivel*) anterior->info)->d != NULL) {
                ((PonteiroDestrutivel*) anterior->info)->d(anterior->info);
            }
            
            free(anterior);
        }

        anterior = atual;
        atual = atual->prox;
    }

    ((PonteiroDestrutivel*) anterior->info)->d(anterior->info);
    free(anterior);
}