#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "basics.h"
#include "error.h"
#include "tokenlexico.h"
#include "analexico.h"

static Token* storedToken = NULL;
static ErrorPosition currentPosition = {0, 0};
ErrorPosition filePosition = {0, 0};

void readNextToken(FILE* currentFile) {
    currentPosition.linha = filePosition.linha;
    currentPosition.coluna = filePosition.coluna;

    string currentIdentifier = (string) malloc(CAP_SIZE_IDENTIFIER);
    associarPonteirosParaErros(currentIdentifier);

    if (currentIdentifier != NULL) {
        memset(currentIdentifier, '\0', CAP_SIZE_IDENTIFIER);
    }
    else {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial");
    }

    if (feof(currentFile)) {
        storedToken = &tokenEof;
        return;
    }

    char currentCharacter;

    while (storedToken != &tokenEof)
    {
        currentCharacter = (char) fgetc(currentFile);

        if (currentCharacter == EOF || isspace(currentCharacter)) {
            storedToken = &;
            return;
        }
        else {
            string currentSpecialToken = (string) malloc(CAP_SIZE_IDENTIFIER);

            Token* currentSpecial = NULL;
            int currentSpecialSize = 0;

            if (currentSpecialToken == NULL) {
                sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial");
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
            sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Identificador Excede o Tamanho Máximo de Caracteres (36)");
        }
    }
}

Token* getCurrentToken() {
    return storedToken;
}
