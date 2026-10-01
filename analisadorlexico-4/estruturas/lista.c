#include <stdlib.h>

#include "lista.h"

void construirLista(Lista* listaParaConstruir, int (*c) (Elemento a, Elemento b), void (*m) (Elemento a), void (*d) (Elemento a)) {
    listaParaConstruir->inicio = NULL;
    listaParaConstruir->fim = NULL;

    listaParaConstruir->count = 0;

    listaParaConstruir->comparar = c;
    listaParaConstruir->mostrar = m;
    listaParaConstruir->destruir = d;
}

void destruirLista(Lista* listaParaDestruir) {
    NoLista* atual = listaParaDestruir->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL) {
        anterior = atual;
        atual = atual->prox;

        if (listaParaDestruir->destruir != NULL) {
            (listaParaDestruir->destruir) (anterior->info);
        }
        free(anterior);
    }

    listaParaDestruir->count = 0;

    listaParaDestruir->inicio = NULL;
    listaParaDestruir->fim = NULL;
}

bool inserirElementoNoFinalLista(Lista* listaParaInserir, Elemento adicionarElemento) {
    NoLista* novoNo = (NoLista*) malloc(sizeof(NoLista));
    if (novoNo == NULL) {
        return false;
    }

    novoNo->info = adicionarElemento;
    novoNo->prox = NULL;

    if (listaParaInserir->inicio == NULL) {
        listaParaInserir->inicio = novoNo;
    }
    else {
        listaParaInserir->fim->prox = novoNo;
    }

    listaParaInserir->fim = novoNo;

    return true;
}

// Depende de função de comparação implementada
bool inserirElementoEmOrdemLista(Lista* listaParaInserir, Elemento adicionarElemento) {
    if (listaParaInserir->comparar == NULL) {
        return false;
    }

    NoLista* novoNo = (NoLista*) malloc(sizeof(NoLista));
    if (novoNo == NULL) {
        return false;
    }

    novoNo->info = adicionarElemento;
    novoNo->prox = NULL;

    NoLista* atual = listaParaInserir->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL) {
        if ((listaParaInserir->comparar) (atual->info, adicionarElemento) == 0) {
            free(novoNo);

            return false;
        }

        anterior = atual;
        atual = atual->prox;
    }

    if (anterior == NULL) {
        listaParaInserir->inicio = novoNo;
        listaParaInserir->fim = novoNo;
    }
    else {
        anterior->prox = novoNo;

        if (anterior == listaParaInserir->fim) {
            listaParaInserir->fim = novoNo;
        }
    }

    novoNo->prox = atual;
    listaParaInserir->count ++;

    return true;
}

bool removerElementoLista(Lista* listaParaRemover, Elemento removerElemento) {
    NoLista* atual = listaParaRemover->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL) {
        if ((listaParaRemover->comparar) (removerElemento, atual->info) == 0) {
            if (anterior == NULL) {
                listaParaRemover->inicio = atual->prox;
            }
            else if (atual == listaParaRemover->fim) {
                listaParaRemover->fim = anterior;
                anterior->prox = NULL;
            }
            else {
                anterior->prox = atual->prox;
            }

            if (listaParaRemover->destruir != NULL) {
                (listaParaRemover->destruir) (atual->info);
            }
            free(atual);

            listaParaRemover->count --;

            return true;
        }

        anterior = atual;
        atual = atual->prox;
    }

    return false;
}

Elemento buscarElementoLista(Lista* listaParaBuscar, Elemento buscarElemento) {
    NoLista* atual = listaParaBuscar->inicio;

    while (atual != NULL) {
        if ((listaParaBuscar->comparar) (buscarElemento, atual->info) == 0) {
            return atual->info;
        }

        atual = atual->prox;
    }

    return NULL;
}

void printarLista(Lista* listaParaPrintar) {
    NoLista* atual = listaParaPrintar->inicio;

    while (atual != NULL) {
        listaParaPrintar->mostrar(atual->info);
        atual = atual->prox;
    }
}
