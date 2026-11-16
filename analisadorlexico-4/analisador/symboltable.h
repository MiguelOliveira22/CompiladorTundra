#ifndef symboltable
#define symboltable

#include "tokenlexico.h"
#include "basics.h"
#include "lista.h"

#define NOME_COMPONENTE_SYMTABLE "[symtable]"

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

typedef struct {
    SymbolTipo tipo;
    SymbolCategoria categoria;
    string identificador;
    
    int escopo;
    int deslocamento;
    bool inicializado;
    bool acessado;

    Lista* parametros;
} Symbol;

extern Token symbolTiposDefinition[TYPE_BASE_BOOLEAN + 1];
extern Lista* customTiposDefinition;

#endif
