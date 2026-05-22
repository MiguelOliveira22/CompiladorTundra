#include <stdio.h>

#include "lista.h"

void construtorListaLigada(ListaLigada* listaNova, int (*comp) (Elemento a, Elemento b)) {
    listaNova->inicio = NULL;
    listaNova->fim = NULL;
    listaNova->quantosElementos = 0;
    
    listaNova->comparar = comp;
}

void destrutorListaLigada(ListaLigada* listaLiberar) {
    
}



void inserirElemento(Elemento novoElemento) {
    
}

void removerElemento(Elemento removerElemento) {
    
}

void buscarElemento(Elemento buscarElemento) {
    
}
