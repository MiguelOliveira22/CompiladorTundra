#ifndef lista
#define lista

typedef void* Elemento;

typedef struct NoListaLigada {
    struct NoListaLigada* next;
    struct NoListaLigada* prev;
    
    Elemento info;
} NoListaLigada;

typedef struct {
    struct NoListaLigada* inicio;
    struct NoListaLigada* fim;
    int quantosElementos;
    
    int (*comparar) (Elemento a, Elemento b);
} ListaLigada;

void construtorListaLigada(ListaLigada* listaNova, int (*comp) (Elemento a, Elemento b));
void destrutorListaLigada(ListaLigada* listaLiberar);

void inserirElemento(Elemento novoElemento);
void removerElemento(Elemento removerElemento);
void buscarElemento(Elemento buscarElemento);

#endif