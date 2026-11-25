#ifndef MEMORYMNG_H
#define MEMORYMNG_H

#include "lista.h"
#include "tokenlexico.h"
#include "symboltable.h"

#define NOME_COMPONENTE_MEMORYMNG "[memorymng]"

typedef struct {
    Elemento dados;
    void (*d) (Elemento a);
} PonteiroDestrutivel;

Symbol* adicionarSymbolTable(string symbolIdentificador, SymbolCategoria symbolCategoria, SymbolTipo symbolTipo);
// extern Lista* customTiposDefinition;
Symbol* buscarSymbolTable(string symbolIdentificador, int escopo);
void removerEscopoSymbolTable(int symbolEscopo);

Token* cadastrarTokenToClose(TokenTipo codigo, string conteudo, bool especial);
void closePonteiros();

#endif