#ifndef lista
#define lista

typedef void* Elemento;

typedef struct NoLista {
    struct NoLista* prox;

    Elemento info;
} NoLista;

typedef struct {
    struct NoLista* raiz;
    struct NoLista* fim;

    struct NoLista* atual;
    int count;
    
    int  (*comparar) (Elemento a, Elemento b);
    void (*mostrar)  (Elemento x);
} Lista;

#endif