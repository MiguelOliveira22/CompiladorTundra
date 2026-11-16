#ifndef analexico
#define analexico

#include "error.h"
#include "tokenlexico.h"

#define CAP_SIZE_IDENTIFIER (36 * sizeof(char))
#define CAP_SIZE_LINHA (256 * sizeof(char))
#define NOME_COMPONENTE_LEXICO "[analexico]"

Token* getCurrentToken(void);
Token* readNextToken(FILE* currentFile);

void readLineToEndError(FILE* currentFile);

extern ErrorPosition currentPosition;
extern char currentLine[CAP_SIZE_LINHA];

#endif
