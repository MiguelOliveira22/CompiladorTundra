#include <stdlib.h>

#include "symboltable.h"
#include "tokenlexico.h"
#include "basics.h"

static Lista* symbolTableContents = NULL;

Token symbolTiposDefinition[] = {
    { TYPE_BASE_NULL,            "null",    false },
    { TYPE_BASE_INTEGER,         "integer", false },
};


