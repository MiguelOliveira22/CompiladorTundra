#include <stdio.h>
#include <stdlib.h>

#include "lista.h"
#include "basics.h"

void contructNewLista(Lista* novaLista, int (*comparar) (Elemento a, Elemento b), void (*mostrar) (Elemento x), void (*destruir) (Elemento y)) {
    novaLista->atual = NULL;
    novaLista->raiz  = NULL;

    novaLista->count = 0;

    novaLista->comparar = comparar;
    novaLista->mostrar  = mostrar;
    novaLista->destruir = destruir;
}

void destructAssignedLista(Lista* assignedLista) {
    NoLista* anterior = NULL;
    assignedLista->atual = assignedLista->raiz;

    while (assignedLista->atual != NULL) {
        anterior = assignedLista->atual;
        assignedLista->atual = assignedLista->atual->prox;

        if (assignedLista->destruir == NULL) {
            (assignedLista->destruir) (anterior->info);
        }
        free(anterior);
    }
}

bool inserirElementoLista(Lista* listaOpera, Elemento novoElemento) {
    NoLista* novoNo;
    novoNo = (NoLista*) malloc(sizeof(NoLista));

    if (novoNo == NULL) {
        return false;
    }

    novoNo->info = novoElemento;
    novoNo->prox = NULL;

    NoLista* anterior = NULL;
    listaOpera->atual = listaOpera->raiz;

    while (listaOpera->atual != NULL) {
        if ((listaOpera->comparar) (listaOpera->atual->info, novoElemento) == 0) {
            free(novoNo);

            return false;
        }

        anterior = listaOper a->atual;
        listaOpera->atual = listaOpera->atual->prox;
    }

    if (anterior == NULL) {
        listaOpera->raiz = novoNo;
    }
    else {
        anterior->prox = novoNo;
    }

    listaOpera->count ++;

    return true;
}

bool removerElementoLista(Lista* listaOpera, Elemento removeElemento) {
    NoLista* anterior = NULL;
    listaOpera->atual = listaOpera->raiz;

    while (listaOpera->atual != NULL) {
        if ((listaOpera->comparar) (listaOpera->atual->info, removeElemento) == 0) {
            if (anterior == NULL) {
                listaOpera->raiz = listaOpera->atual->prox;
            }
            else {
                anterior->prox = listaOpera->atual->prox;
            }

            if (listaOpera->destruir == NULL) {
                (listaOpera->destruir) (listaOpera->atual->info);
            }
            free(listaOpera->atual);

            listaOpera->count --;

            return true;
        }

        anterior = listaOpera->atual;
        listaOpera->atual = listaOpera->atual->prox;
    }
    
    return false;
}

// Retorna NULL se não encontrar. Do contrário, retorna o Elemento.
Elemento buscarElementoLista(Lista* listaOpera, Elemento buscarElemento) {
    listaOpera->atual = listaOpera->raiz;

    while (listaOpera->atual != NULL) {
        if ((listaOpera->comparar) (listaOpera->atual->info, buscarElemento) == 0) {
            return listaOpera->atual->info;
        }

        listaOpera->atual = listaOpera->atual->prox;
    }

    return NULL;
}