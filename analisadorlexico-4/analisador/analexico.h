#ifndef analexico
#define analexico

#include "tokenlexico.h"

#define CAP_SIZE_IDENTIFIER (36 * sizeof(char))
#define NOME_COMPONENTE_LEXICO "[analexico]"

Token* getCurrentToken(void);
Token* readNextToken(FILE* currentFile);

#endif
