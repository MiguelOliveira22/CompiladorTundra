#ifndef analexico
#define analexico

#include "basics.h"
#include "lista.h"
#include "error.h"
#include "tokenlexico.h"

#define CAP_SIZE_IDENTIFIER (36 * sizeof(char))

void   readNextToken(FILE* currentFile);
Token* getCurrentToken(void);
Token* getNextToken(FILE* currentFile);

#endif