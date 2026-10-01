#ifndef lista
#define lista

#include "basics.h"

typedef struct NoLista {
    Elemento info;
    
    struct NoLista* prox;
} NoLista;

typedef struct {
    NoLista* inicio;
    NoLista* fim;
    int count;

    int (*comparar) (Elemento a, Elemento b);
    void (*mostrar) (Elemento a);
    void (*destruir) (Elemento a);
} Lista;

void construirLista(Lista* listaParaConstruir, int (*c) (Elemento a, Elemento b), void (*m) (Elemento a), void (*d) (Elemento a));
void destruirLista(Lista* listaParaDestruir);

bool inserirElementoNoFinalLista(Lista* listaParaInserir, Elemento adicionarElemento);
bool inserirElementoEmOrdemLista(Lista* listaParaInserir, Elemento adicionarElemento);
bool removerElementoLista(Lista* listaParaRemover, Elemento removerElemento);
Elemento buscarElementoLista(Lista* listaParaBuscar, Elemento buscarElemento);
void printarLista(Lista* listaParaPrintar);

#endif