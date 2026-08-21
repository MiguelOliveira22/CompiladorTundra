#include <stdlib.h>

#include "basics/lista.h"

/*
 * C -> Função para comparar elementos.
 * M -> Função para escrever elementos no print.
 * D -> Função para dealocar elementos.
 */
void construirNovaLista(Lista* listaParaConstruir, int (*c) (Elemento a, Elemento b), void (*m) (Elemento a), void (*d) (Elemento a)) {
    listaParaConstruir->inicio = NULL;
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
}

bool inserirNovoElemento(Lista* listaParaInserir, Elemento adicionarElemento) {
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
    }
    else {
        anterior->prox = novoNo;
    }

    novoNo->prox = atual;
    listaParaInserir->count ++;

    return true;
}

// TODO
bool removerElemento(Lista* listaParaRemover, Elemento removerElemento) {

}
Elemento buscarElemento(Lista* listaParaBuscar, Elemento buscarElemento) {

}
