#ifndef memory
#define memory

#include "tokenlexico.h"

typedef struct {
    Elemento dados;
    void (*d) (Elemento a);
} PonteiroDestrutivel;

void cadastrarStringsToClose(string content);
void closeStrings();

void cadastrarNumerosToClose(Token* number);
void closeNumeros();

void cadastrarIdentifiersToClose(Token* identifier);
void closeIdentifiers();

#endif