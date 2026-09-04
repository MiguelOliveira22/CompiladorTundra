#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "basics.h"
#include "error.h"
#include "tokenlexico.h"
#include "analexico.h"

Token* storedToken = NULL;

ErrorPosition currentPosition;
ErrorPosition filePosition;

void readNextToken(FILE* currentFile) {
    currentPosition.linha = filePosition.linha;
    currentPosition.coluna = filePosition.coluna;

    string currentIdentifier = (string) malloc(sizeof(char) * CAP_SIZE_IDENTIFIER);
    if (currentIdentifier != NULL) {
        memset(currentIdentifier, '\0', sizeof(char) * CAP_SIZE_IDENTIFIER);
    }
    else {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial", currentFile);
        return;
    }

    if (feof(currentFile)) {
        storedToken = &tokenDefinitions[TOKEN_OPER_EOF];
        return;
    }

    char currentCharacter;
    while (storedToken != &tokenDefinitions[TOKEN_OPER_EOF])
    {
        currentCharacter = (char) fgetc(currentFile);
        if (currentCharacter == EOF || isspace(currentCharacter)) {
            storedToken = &;
            return;
        }
        else {
            string currentSpecialToken = (string) malloc(sizeof(char) * CAP_SIZE_IDENTIFIER);

            Token* currentSpecial = NULL;
            int currentSpecialSize = 0;

            if (currentIdentifier == NULL) {
                sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial", currentFile);
                return;
            }

            for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i++) {
                memset(currentIdentifier, '\0', sizeof(char) * CAP_SIZE_IDENTIFIER);
                fgets(currentSpecialToken, strlen(tokenDefinitions[i].identificador), currentFile);
                
                if (strcmp(currentSpecialToken, tokenDefinitions[i].identificador) == 0 && tokenDefinitions[i].tokenEspecial) {
                    if (strlen(tokenDefinitions[i].identificador) > currentSpecialSize) {
                        currentSpecial = &tokenDefinitions[i];
                        currentSpecialSize = strlen(tokenDefinitions[i].identificador);
                    }
                }
            }

            if (currentSpecial != NULL) {
                storedToken = &;
            }

            return;
        }

        if (currentCharacter == '\n') {
            filePosition.linha = 0;
            filePosition.coluna += 1;
        }
        else {
            filePosition.linha += 1;
        }

        if (strlen(currentIdentifier) + 1 < CAP_SIZE_IDENTIFIER) {
            strcat(currentIdentifier, &currentCharacter);
        }
        else {
            sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Identificador Excede o Tamanho Máximo de Caracteres (36)", currentFile);
            return;
        }
    }
}

Token* getCurrentToken() {
    return storedToken;
}

// const type& nome; -> só pra ler com const, pq n copia e n altera (acessa por algo similar a deferencia)
// type* nome; -> por referencia, o ponto é a cessar diretamente, ent não alterar é bobo
// type nome; -> por valor copia, ent n precisa se preocupar com alterar