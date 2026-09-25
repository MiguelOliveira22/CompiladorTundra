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

    NoLista* atual = ponteirosParaDestruir->inicio;
    while (atual != NULL) {

        atual = atual->info;
    }

    destruirLista(ponteirosParaDestruir);
}

void associarPonteirosParaErros(Elemento ponteiro, void (*d) (Elemento a)) {
    if (ponteirosParaDestruir == NULL) {
        construirLista(ponteirosParaDestruir, NULL, NULL, NULL);
    }

    NoLista* atual = ponteirosParaDestruir->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL) {
        anterior = atual;
        atual = atual->prox;
    }

    NoLista* novoNo;
    novoNo = (NoLista*) malloc(sizeof(NoLista));

    novoNo->info = { ponteiro, destrutor };
    novoNo->prox = NULL;

    anterior->prox = novoNo;
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
