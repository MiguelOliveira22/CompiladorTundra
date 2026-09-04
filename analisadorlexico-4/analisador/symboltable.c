#include <stdlib.h>

#include "symboltable.h"
#include "tokenlexico.h"
#include "basics.h"

Token symbolTiposDefinition[] = {
    { TYPE_BASE_NULL,            "null",    false },
    { TYPE_BASE_INTEGER,         "integer", false },
};

void startSymbolTableList() {
    
}