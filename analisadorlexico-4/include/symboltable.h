#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "tokenlexico.h"
#include "basics.h"
#include "lista.h"

#define NOME_COMPONENTE_SYMTABLE "[symtable]"

typedef enum {
    TYPE_BASE_NULL,
    TYPE_BASE_INTEGER,
    TYPE_BASE_BOOLEAN,

    XINTERNAL_LIMIT_TYPE_BASE,
} SymbolTipo;

typedef enum {
    CATG_BASE_NOME_PROGRAMA,
    CATG_BASE_VARIAVEL,
    CATG_BASE_TIPO,
    CATG_BASE_ROTULO,
    CATG_BASE_PROCEDIMENTO,
    CATG_BASE_FUNCAO,

    XINTERNAL_LIMIT_CATG_BASE,
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

extern Token symbolTiposDefinition[XINTERNAL_LIMIT_TYPE_BASE];

#endif
