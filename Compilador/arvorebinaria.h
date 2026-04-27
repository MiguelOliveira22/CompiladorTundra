#ifndef arvorebinaria

#define arvorebinaria

typedef void* Elemento;

typedef struct NoArvoreBinaria {
    struct NoArvoreBinaria* esq;
    struct NoArvoreBinaria* dir;
    
    Elemento info;
} NoArvoreBinaria;

typedef struct {
    struct NoArvoreBinaria* raiz;
    
    int  (*comparar) (Elemento a, Elemento b);
    void (*mostrar)  (Elemento x);
} ArvoreBinaria;

#endif