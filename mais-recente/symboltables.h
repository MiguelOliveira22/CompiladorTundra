#ifndef symboltables
#define symboltables

typedef enum {
    integer,
    none,
} TipoSimbolo;

typedef enum {
    variable,
    parameter,
    procedure,
    function,
} NaturezaSimbolo;

extern const char*[] tiposSimbolo;

extern const char*[] naturezasSimbolos;

typedef struct {
    char* identificadorSimbolo;
    int   tipoSimbolo;
    int   naturezaSimbolo;
    int   escopoSimbolo;
    // int enderecoSimbolo; -> TabSin
} symbolTable;

#endif