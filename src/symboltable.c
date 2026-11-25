#include "symboltable.h"
#include "tokenlexico.h"
#include "basics.h"

Token symbolTiposDefinition[] = {
    { (TokenTipo) TYPE_BASE_NULL,    "null",    false },
    { (TokenTipo) TYPE_BASE_INTEGER, "integer", false },
    { (TokenTipo) TYPE_BASE_BOOLEAN, "boolean", false },
};