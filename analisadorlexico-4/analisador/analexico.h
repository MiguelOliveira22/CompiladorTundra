#ifndef analexico
#define analexico

#include "basics.h"
#include "lista.h"
#include "error.h"
#include "tokenlexico.h"

const int CAP_SIZE_IDENTIFIER = 36 * sizeof(char);

void   readNextToken(FILE* currentFile);
Token* getCurrentToken();

#endif