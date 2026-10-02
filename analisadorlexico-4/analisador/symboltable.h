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
    CATG_BASE_NOME_PROGRAMA,
    CATG_BASE_VARIAVEL,
    CATG_BASE_TIPO,
    CATG_BASE_ROTULO,
    CATG_BASE_PROCEDIMENTO,
    CATG_BASE_FUNCAO
} SymbolCategoria;

typedef struct SymbolParametro {
    SymbolTipo tipo;
    struct SymbolParametro* proximo;
} SymbolParametro;

typedef struct {
    string symbolIdentificador;
    Token* symbolCategoria;
    SymbolTipo symbolTipo;
    Token* referenciaNatureza;
    i8 symbolEscopo;
    bool symbolIdentificadorAlocado;

    SymbolParametro* symbolParameters;
} SymbolGenerico;

extern Token symbolTiposDefinition[TYPE_BASE_BOOLEAN + 1];
extern Lista* customTiposDefinition;

void adicionarSymbolTable(string symbolIdentificador, Token* symbolCategoria, SymbolTipo symbolTipo, Token* referenciaNatureza, i8 symbolEscopo);
SymbolGenerico* buscarSymbolTable(string symbolIdentificador);
Token* buscarTokenTipoBase(string nomeTipo);
void removerEscopoSymbolTable(i8 symbolEscopo);

#endif
