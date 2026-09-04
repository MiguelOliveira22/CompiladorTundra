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

void construirNovaLista(Lista* listaParaConstruir, int (*c) (Elemento a, Elemento b), void (*m) (Elemento a), void (*d) (Elemento a));
void destruirLista(Lista* listaParaDestruir);

bool inserirNovoElemento(Lista* listaParaInserir, Elemento adicionarElemento);
bool removerElemento(Lista* listaParaRemover, Elemento removerElemento);
Elemento buscarElemento(Lista* listaParaBuscar, Elemento buscarElemento);

#endif