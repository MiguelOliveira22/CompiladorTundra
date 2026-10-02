#ifndef memorymng
#define memorymng

#include "tokenlexico.h"

typedef struct {
    Elemento dados;
    void (*d) (Elemento a);
} PonteiroDestrutivel;

void cadastrarIdentifiersToClose(Token* identifier);
void closeIdentifiers();

void cadastrarNumerosToClose(Token* number);
void closeNumeros();

void cadastrarStringsToClose(string content);
void closeStrings();

void cadastrarPonteirosToClose(Elemento ponteiro, void (*d) (Elemento a));
void closePonteiros();

#endif