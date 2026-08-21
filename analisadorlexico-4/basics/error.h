#ifndef error
#define error

#include "basics/basics.h"
#include "basics/lista.h"

typedef enum {
    ERROR_OK_OPERATION_SUCCESS,

    ERROR_FILE_NO_PROGRAM_FILES,
    ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED,

    
} ErrorTipos;

int sairErroTerminal(ErrorTipos erro, string message, Lista* filesToClose);

#endif