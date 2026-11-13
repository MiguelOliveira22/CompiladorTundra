#ifndef symboltable
#define symboltable

#include "tokenlexico.h"
#include "basics.h"
#include "lista.h"

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
    const char* nome;
    SymbolTipo tipo;
    bool porReferencia;
    struct SymbolParametro* proximo;
} SymbolParametro;

typedef struct {
    string symbolIdentificador;
    Token* symbolCategoria;
    SymbolTipo symbolTipo;
    Token* referenciaNatureza;
    i8 symbolEscopo;
    bool symbolIdentificadorAlocado;
    int symbolDeslocamento;
    bool symbolParametroPorReferencia;
    bool symbolBuiltinIO;
    int symbolIndiceParametro;
    int symbolQuantidadeParametros;
    int symbolDeslocamentoResultado;

    SymbolParametro* symbolParameters;
} SymbolGenerico;

extern Token symbolTiposDefinition[TYPE_BASE_BOOLEAN + 1];
extern Lista* customTiposDefinition;

void adicionarSymbolTable(string symbolIdentificador, Token* symbolCategoria, SymbolTipo symbolTipo, Token* referenciaNatureza, i8 symbolEscopo);
SymbolGenerico* buscarSymbolTable(string symbolIdentificador, i8 escopo);
Token* buscarTokenTipoBase(string nomeTipo);
void removerEscopoSymbolTable(i8 symbolEscopo);

#endif
