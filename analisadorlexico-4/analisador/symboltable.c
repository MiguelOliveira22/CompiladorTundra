#include <stdlib.h>

#include "analisador/symboltable.h"
#include "analisador/tokenlexico.h"
#include "basics/basics.h"

Token symbolTiposDefinition[] = {
    { TYPE_BASE_NULL,            "null",    false },
    { TYPE_BASE_INTEGER,         "integer", false },
};