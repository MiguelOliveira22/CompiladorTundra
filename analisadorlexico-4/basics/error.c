#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "lista.h"

static Lista* ponteirosParaDestruir = NULL;

typedef struct {
    Elemento dados;
    void (*d) (Elemento a);
} PonteiroDestrutivel;

static void destruirPonteiros() {
    if (ponteirosParaDestruir == NULL) {
        return;
    }

    destruirLista(ponteirosParaDestruir);
}

void associarPonteirosParaErros(Elemento ponteiro, void (*d) (Elemento a)) {
    if (ponteirosParaDestruir == NULL) {
        ponteirosParaDestruir = (Lista*) malloc(sizeof(Lista));
        construirLista(ponteirosParaDestruir, NULL, NULL, NULL);
    }

    NoLista* atual = ponteirosParaDestruir->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL) {
        anterior = atual;
        atual = atual->prox;
    }

    NoLista* novoNo = (NoLista*) malloc(sizeof(NoLista));
    PonteiroDestrutivel* novoDado = (PonteiroDestrutivel*) malloc(sizeof(PonteiroDestrutivel));
    novoDado->dados = ponteiro;
    novoDado->d = d;
    novoNo->info = novoDado;
    novoNo->prox = NULL;

    if (anterior == NULL) {
        ponteirosParaDestruir->inicio = novoNo;
    } else {
        anterior->prox = novoNo;
    }
    ponteirosParaDestruir->count++;
}

void sairErroTerminal(ErrorTipos erro, string message)
{
    destruirPonteiros();

    if (erro == ERROR_OK_OPERATION_SUCCESS) {
        printf("OK Sucesso - %s", message);
    }
    else if (strlen(message) > 0) {
        printf("Erro %d - %s", erro, message);
    }
    else {
        printf("Erro %d - Encerrando Execução", erro);
    }

    exit(erro);
}
