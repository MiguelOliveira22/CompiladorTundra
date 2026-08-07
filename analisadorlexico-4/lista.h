#ifndef lista
#define lista

#include "basics.h"

typedef void* Elemento;

typedef struct NoLista {
    struct NoLista* prox;

    Elemento info;
} NoLista;

typedef struct {
    struct NoLista* raiz;
    struct NoLista* atual;
    i8 count;
    
    int  (*comparar) (Elemento a, Elemento b);
    void (*mostrar)  (Elemento x);
    void (*destruir) (Elemento y);
} Lista;

void contructNewLista(Lista* novaLista, int (*comparar) (Elemento a, Elemento b), void (*mostrar) (Elemento x), void (*destruir) (Elemento y));
void destructAssignedLista(Lista* assignedLista);

bool inserirElementoLista(Lista* listaOpera, Elemento novoElemento);
bool removerElementoLista(Lista* listaOpera, Elemento removeElemento);
Elemento buscarElementoLista(Lista* listaOpera, Elemento buscarElemento);

#endif