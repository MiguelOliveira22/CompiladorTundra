#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

#include "compiladortundra.h"
#include "basics.h"
#include "error.h"
#include "tokenlexico.h"
#include "analexico.h"
#include "memorymng.h"

// Posição Leitura Global + Linha Atual
CompilationPosition currentPosition = {0, 0};
static CompilationPosition filePosition = {0, 0};

char currentLine[CAP_SIZE_LINHA_BUFFER] = { '\0' };
static int posCurrentLine = 0;

static Token* storedToken = NULL;
static char currentIdentifier[CAP_SIZE_IDENTIFIER];

static void atualizarPosicao(const char currentChar) {
    if (currentChar == '\n') {
        filePosition.coluna = 0;
        filePosition.linha ++;

        posCurrentLine = 0;
        currentLine[posCurrentLine] = '\0';
    }
    else {
        filePosition.coluna ++;
    }
}

static void atualizarLinha(char currentChar) {
    if (posCurrentLine + 1 < CAP_SIZE_LINHA_BUFFER) {
        currentLine[posCurrentLine] = currentChar;
        currentLine[posCurrentLine + 1] = '\0';

        posCurrentLine ++;
    }
    else if (posCurrentLine < CAP_SIZE_LINHA_BUFFER) {
        currentLine[CAP_SIZE_LINHA_BUFFER - 4] = '.';
        currentLine[CAP_SIZE_LINHA_BUFFER - 3] = '.';
        currentLine[CAP_SIZE_LINHA_BUFFER - 2] = '.';

        posCurrentLine ++;
    }
}

static Token* getIdentifierValid(char* word) {
    int wordSize = strlen(word);
    if (wordSize == 0) { return NULL; }
    
    for (int i = 0; i < wordSize; i ++) {
        if (word[i] == '_' && i != 0) { continue; }
        if ((word[i] >= '0' && word[i] <= '9') && i != 0) { continue; }
        if (word[i] >= 'A' && word[i] <= 'Z') { continue; }
        if (word[i] >= 'a' && word[i] <= 'z') { continue; }
        
        return NULL;
    }
    
    Token* identifier = cadastrarTokenToClose(TOKEN_OPER_IDENTIFICADOR, word, false);
    return identifier;
}

static Token* getNumericValid(char* word) {
    int wordSize = strlen(word);
    
    if (wordSize == 0) { return NULL; }
    
    int number = 0;
    for (int i = 0; i < wordSize; i ++) {
        if (word[i] < '0' || word[i] > '9') {
            return NULL;
        }

        if (number > INT_MAX / 10) {
            sairErroTerminal(ERROR_NUMERIC_OVERFLOW, "Valor numérico excede o tamanho máximo de representação do inteiro");
        }

        number *= 10;

        int somaAtual = (int) (word[i] - '0');
        if (number > INT_MAX - somaAtual) {
            sairErroTerminal(ERROR_NUMERIC_OVERFLOW, "Valor numérico excede o tamanho máximo de representação do inteiro");
        }

        number += somaAtual;
    }

    Token* identifier = cadastrarTokenToClose(TOKEN_OPER_NUMERO, word, false);
    return identifier;
}

static Token* procurarDefinicao(string candidato, bool especial, int tamanho) {
    for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i ++) {
        if (tokenDefinitions[i].tokenEspecial != especial) {
            continue;
        }

        if (strlen(tokenDefinitions[i].identificador) != tamanho) {
            continue;
        }

        if (strcmp(candidato, tokenDefinitions[i].identificador) != 0) {
            continue;
        }

        return &tokenDefinitions[i];
    }

    return NULL;
}

static Token* getToken(string word) {
    Token* bufferToken;

    bufferToken = procurarDefinicao(word, false, strlen(word));
    if (bufferToken != NULL) {
        return bufferToken;
    }

    bufferToken = procurarDefinicao(word, true, strlen(word));
    if (bufferToken != NULL) {
        return bufferToken;
    }

    bufferToken = getIdentifierValid(word);
    if (bufferToken != NULL) {
        return bufferToken;
    }

    bufferToken = getNumericValid(word);
    if (bufferToken != NULL) {
        return bufferToken;
    }

    return &tokenInvalido;
}

static void findNextToken(FILE* currentFile) {
    memset(currentIdentifier, '\0', CAP_SIZE_IDENTIFIER);
    char currentChar = 0;
    
    while (true) {
        currentChar = fgetc(currentFile);

        if (isspace(currentChar) || feof(currentFile)) {
            atualizarLinha(currentChar);
            atualizarPosicao(currentChar);

            if (strlen(currentIdentifier) > 0) {
                storedToken = getToken(currentIdentifier);
                return;
            }

            if (feof(currentFile)) {
                storedToken = &tokenEof;

                filePosition.linha = 0;
                filePosition.coluna = 0;

                return;
            }

            continue;
        }

        char simples[2] = { currentChar, '\0' };
        Token* foundSpecial;

        foundSpecial = procurarDefinicao(simples, true, 1);
        if (foundSpecial != NULL) {
            char proximoChar = fgetc(currentFile);
            char duplo[3] = { currentChar, proximoChar, '\0' };
            Token* foundSpecialDuplo;

            foundSpecialDuplo = procurarDefinicao(duplo, true, 2);
            if (foundSpecialDuplo != NULL) {
                foundSpecial = foundSpecialDuplo;
            }

            if (strlen(currentIdentifier) > 0) {
                ungetc(proximoChar, currentFile);
                ungetc(currentChar, currentFile);

                storedToken = getToken(currentIdentifier);
                return;
            }
            else {
                atualizarLinha(currentChar);
                atualizarPosicao(currentChar);

                if (foundSpecial == foundSpecialDuplo) {
                    atualizarLinha(proximoChar);
                    atualizarPosicao(proximoChar);
                }
                else {
                    ungetc(proximoChar, currentFile);
                }

                storedToken = foundSpecial;
                return;
            }
        }

        int tamanho = strlen(currentIdentifier);
        if (tamanho + 1 >= CAP_SIZE_IDENTIFIER) {
            sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Identificador Excede o Tamanho Máximo de Caracteres (35)");
        }

        currentIdentifier[tamanho] = currentChar;
        atualizarLinha(currentChar);
        atualizarPosicao(currentChar);
    }
}

Token* getCurrentToken() {
    currentPosition.linha = filePosition.linha;
    currentPosition.coluna = filePosition.coluna;

    // printf("%s - %s\n", NOME_COMPONENTE_LEXICO, storedToken->identificador);

    return storedToken;
}

Token* readNextToken(FILE* currentFile) {
    while (true) {
        findNextToken(currentFile);

        if (storedToken->codigo == TOKEN_SYMB_ABRECOMENTARIO) {
            while (storedToken->codigo != TOKEN_SYMB_FECHACOMENTARIO) {
                findNextToken(currentFile);

                if (storedToken->codigo == TOKEN_OPER_EOF) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Comentário Não Foi Fechado Antes do Fim do Arquivo");
                }
            }

            findNextToken(currentFile);
        }

        return getCurrentToken();
    }
}

void readLineToEndError(FILE* currentFile) {
    while (true) {
        char currentChar = (char) fgetc(currentFile);

        if (currentChar == '\n' || feof(currentFile)) {
            return;
        }

        atualizarLinha(currentChar);
    }
}
