#ifndef symboltable
#define symboltable

#include "tokenlexico.h"
#include "basics.h"

typedef enum {
    TYPE_BASE_NULL,
    TYPE_BASE_INTEGER,
    TYPE_BASE_BOOLEAN,
} SymbolTipo;

typedef enum {
    CATG_BASE_PROGRAM_NAME,
    CATG_BASE_VARIABLE,
    CATG_BASE_
} SymbolCategoria;

typedef struct {
    string symbolIdentificador;
    Token* symbolTipo;
    SymbolCategoria symbolCategoria;
    i8 symbolEscopo;

    Lista* symbolParameters;
} SymbolGenerico;

extern Token symbolTiposDefinition[TYPE_BASE_BOOLEAN + 1];
extern Lista* customTiposDefinition;

void adicionarSymbolTable(string symbolIdentificador, Token* symbolTipo, SymbolTipo symbolCategoria, Token* referenciaNatureza, i8 symbolEscopo);
SymbolGenerico* buscarSymbolTable(string symbolIdentificador);
Token* buscarTokenTipoBase(string nomeTipo);
void removerEscopoSymbolTable(i8 symbolEscopo);

#endif