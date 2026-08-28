#ifndef analexico
#define analexico

#include "basics/basics.h"
#include "basics/lista.h"
#include "basics/error.h"
#include "analisador/tokenlexico.h"

const int CAP_SIZE_IDENTIFIER = 36 * sizeof(char);

void   readNextToken(FILE* currentFile);
Token* getCurrentToken();

#endif