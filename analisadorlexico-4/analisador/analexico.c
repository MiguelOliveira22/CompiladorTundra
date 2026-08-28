#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "basics/basics.h"
#include "basics/error.h"
#include "analisador/tokenlexico.h"
#include "analexico.h"

Token* storedToken = NULL;

ErrorPosition currentPosition;
ErrorPosition filePosition;

void readNextToken(FILE* currentFile) {
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
            // getToken
            return;
        }
        else {
            bool foundToken = false;
            for (int i = 0; i < && !foundToken; i++) {
                i8 identifierSize = strlen();
                
                if (strncmp() == 0 && .special) {

                }
            }

            return;
        }
        else {
            for (int i = 0; i < sizeof(tokens) / sizeof(token); i ++) {
                token currentToken = tokens[i];
                
                if (palavra[currentTamanho] == currentToken.lexicoArquivo[0] && currentToken.isEspecial) {
                    if (currentTamanho > 0) {
                        ungetc(palavra[currentTamanho], file);
                        palavra[currentTamanho] = '\0';
                        
                        return getToken(palavra);
                    }
                    else {
                        palavra[currentTamanho + 1] = getc(file);
                        palavra[currentTamanho + 2] = '\0';
                        
                        token partialToken = getToken(palavra);
                        
                        if (partialToken.codigoToken == invalido) {
                            ungetc(palavra[currentTamanho + 1], file);
                            palavra[currentTamanho + 1] = '\0';
                            
                            partialToken = getToken(palavra);
                        }
                        
                        return partialToken;
                    }
                }

        if (currentCharacter == '\n') {
            filePosition.linha = 0;
            filePosition.coluna += 1;
        }
        else {
            filePosition.linha += 1;
        }

        if (strlen(currentIdentifier) + 1 < CAP_SIZE_IDENTIFIER) {
            strcat(currentIdentifier, currentCharacter);
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
