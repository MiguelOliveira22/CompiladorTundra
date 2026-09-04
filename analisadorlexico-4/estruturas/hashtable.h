#ifndef hashtable
#define hashtable

#include "basics.h"

typedef struct {
    Elemento* conteudo;
    int byteCount;
    i8 maxCount;

    void (*destruir) (Elemento a);
    i8   (*getHash)  (Elemento a);
} HashTable;

void construirHashTable(HashTable* hashTable, Elemento* conteudo, int byteCount, i8 maxCount, void (*d) (Elemento a), i8 (*g) (Elemento a));
void destruirHashTable(HashTable* HashTable);

bool inserirElemento(HashTable* hashTable, Elemento adicionarElemento);
bool removerOnIndex(HashTable* hashTable, int index);
Elemento buscarElemento(HashTable* hashTable, int index);

#endif