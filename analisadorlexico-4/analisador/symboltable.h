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
    CATG_BASE_FUNCAO,
    CATG_BASE_PARAMETRO_CHAMADA
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