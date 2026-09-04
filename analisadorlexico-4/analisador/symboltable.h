#ifndef symboltable
#define symboltable

#include "tokenlexico.h"
#include "basics.h"

typedef enum {
    TYPE_BASE_NULL,
    TYPE_BASE_INTEGER,
} SymbolTipo;

typedef struct {
    string symbolIdentificador;
    Token symbolTipo;
    TokenTipo symbolNatureza;
    i8 symbolEscopo;
} SymbolTable;

extern Token symbolTiposDefinition[TYPE_BASE_INTEGER + 1];

#endif