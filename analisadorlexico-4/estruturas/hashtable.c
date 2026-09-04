#include <stdlib.h>

#include "basics.h"
#include "hashtable.h"

void construirHashTable(HashTable* hashTable, Elemento* conteudo, int byteCount, i8 maxCount, void (*d) (Elemento a), i8 (*g) (Elemento a)) {
    hashTable->conteudo = conteudo;
    hashTable->maxCount = maxCount;

    hashTable->byteCount = byteCount;

    hashTable->destruir = d;
    hashTable->getHash  = g;
}
void destruirHashTable(HashTable* hashTable) {
    unsigned char cont[] = (unsigned char**) hashTable->conteudo;

    for (int i = 0; i < hashTable->maxCount; i ++) {
        if ()
        data[i] = cont[index * hashTable->byteCount + i];
    }
}

bool inserirElemento(HashTable* hashTable, Elemento adicionarElemento) {

}
bool removerOnIndex(HashTable* hashTable, int index) {

}
Elemento buscarElemento(HashTable* hashTable, int index) {
    string data = (string) calloc(hashTable->byteCount, sizeof(unsigned char));

    unsigned char cont[] = (unsigned char**) hashTable->conteudo;
    for (int i = 0; i < hashTable->byteCount; i ++) {
        data[i] = cont[index * hashTable->byteCount + i];
    }

    return (Elemento) data;
}