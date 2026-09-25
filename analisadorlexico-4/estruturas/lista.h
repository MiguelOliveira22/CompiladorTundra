#ifndef lista
#define lista

#include "basics.h"

typedef struct NoLista {
    Elemento info;
    
    struct NoLista* prox;
} NoLista;

typedef struct {
    NoLista* inicio;
    i8 count;

    int (*comparar) (Elemento a, Elemento b);
    void (*mostrar) (Elemento a);
    void (*destruir) (Elemento a);
} Lista;

void construirLista(Lista* listaParaConstruir, int (*c) (Elemento a, Elemento b), void (*m) (Elemento a), void (*d) (Elemento a));
void destruirLista(Lista* listaParaDestruir);

bool inserirElemento(Lista* listaParaInserir, Elemento adicionarElemento);
bool removerElemento(Lista* listaParaRemover, Elemento removerElemento);
Elemento buscarElemento(Lista* listaParaBuscar, Elemento buscarElemento);
Elemento buscarElemento(Lista* listaParaBuscar, int index);

#endif