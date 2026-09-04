#ifndef error
#define error

#include "basics.h"
#include "lista.h"

typedef enum {
    ERROR_OK_OPERATION_SUCCESS,

    ERROR_FILE_NO_PROGRAM_FILES,
    ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED,

    ERROR_INSUFFICIENT_MEMORY_MALLOC,
    ERROR_EXCEEDED_IDENTIFIER_SIZE,
    
} ErrorTipos;

typedef struct {
    int linha;
    int coluna;
} ErrorPosition;

void sairErroTerminal(ErrorTipos erro, string message, FILE* fileToClose);

#endif