#ifndef error
#define error

#include "basics.h"

typedef enum {
    ERROR_OK_OPERATION_SUCCESS,

    ERROR_FILE_NO_PROGRAM_FILES,
    ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED,

    ERROR_INSUFFICIENT_MEMORY_MALLOC,
    ERROR_EXCEEDED_IDENTIFIER_SIZE,
    ERROR_NUMERIC_OVERFLOW,
    
    ERROR_UNKNOWN_TOKEN,
    ERROR_INVALID_TOKEN,
    ERROR_UNDECLARED_IDENTIFIER,
    ERROR_IDENTIFIER_ALREADY_DECLARED,
    ERROR_UNKNOWN_TYPE,
    ERROR_TYPE_MISMATCH,
    ERROR_ARGUMENT_COUNT,
    ERROR_INVALID_CALL,

    XINTERNAL_LIMITE_TABELA_ERROS,
} ErrorTipos;

typedef enum {
    AVISO_EXEMPLO,

    XINTERNAL_LIMITE_TABELA_AVISOS,
} AvisoTipos;

typedef struct {
    int linha;
    int coluna;
} ErrorPosition;

void sairErroTerminal(ErrorTipos erro, string message);
void printAvisoTerminal(AvisoTipos aviso, string message);

#endif
