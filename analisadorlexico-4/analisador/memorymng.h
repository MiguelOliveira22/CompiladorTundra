#ifndef memorymng
#define memorymng

#include "lista.h"
#include "tokenlexico.h"
#include "symboltable.h"

#define NOME_COMPONENTE_MEMORYMNG "[memorymng]"

typedef struct {
    Elemento dados;
    void (*d) (Elemento a);
} PonteiroDestrutivel;

Symbol* adicionarSymbolTable(string symbolIdentificador, SymbolCategoria symbolCategoria, SymbolTipo symbolTipo);
Symbol* buscarSymbolTable(string symbolIdentificador, i8 escopo);
void removerEscopoSymbolTable(i8 symbolEscopo);

Token* cadastrarTokenToClose(TokenTipo codigo, string conteudo, bool especial);
void closePonteiros();

#endif