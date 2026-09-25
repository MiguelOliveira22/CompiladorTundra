#include <stdlib.h>

#include "error.h"
#include "basics.h"
#include "hashtable.h"

void construirHashTable(HashTable* hashTable, int byteSize, i8 maxCount, void (*d) (Elemento a), i8 (*g) (Elemento a)) {
    hashTable->conteudo = calloc(maxCount, byteSize);

    if (hashTable->conteudo == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "Memoria Insuficiente Para Alocar A Tabela");
    }

    hashTable->maxCount = maxCount;
    hashTable->byteSize = byteSize;

    hashTable->destruir = d;
    hashTable->getHash  = g;
}

void destruirHashTable(HashTable* hashTable) {
    unsigned char cont[] = (unsigned char**) hashTable->conteudo;

    for (int i = 0; i < hashTable->maxCount; i ++) {
        if ()
        data[i] = cont[index * hashTable->byteSize + i];
    }
}

bool inserirElemento(HashTable* hashTable, Elemento adicionarElemento) {
    i8 index = hashTable->getHash(adicionarElemento);
}
bool removerOnIndex(HashTable* hashTable, int index) {

}
Elemento buscarElemento(HashTable* hashTable, int index) {
    string data = (string) calloc(hashTable->byteSize, sizeof(unsigned char));

    unsigned char cont[] = (unsigned char**) hashTable->conteudo;
    for (int i = 0; i < hashTable->byteSize; i ++) {
        data[i] = cont[index * hashTable->byteSize + i];
    }

    return (Elemento) data;
}