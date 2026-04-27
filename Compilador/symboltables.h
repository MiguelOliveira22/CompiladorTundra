#ifndef symboltables

#define symboltables

typedef enum {
    integer,
    boolean,
    none,
} TipoSimbolo;

typedef enum {
    variable,
    parameter,
    procedure,
    function,
} NaturezaSimbolo;

typedef struct {
    char* identificador;
    int   tipo;
    int   natureza;
    int   escopo;
} SymbolTable;

#endif