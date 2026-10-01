#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "basics.h"
#include "error.h"
#include "tokenlexico.h"
#include "analexico.h"

// Posição Leitura Global
ErrorPosition filePosition = {0, 0};

static Token* storedToken = NULL;
static ErrorPosition currentPosition = {0, 0};

static void atualizarPosicao(const char currentChar) {
    if (currentChar == '\n') {
        filePosition.coluna = 0;
        filePosition.linha ++;
    }
    else {
        filePosition.coluna ++;
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
    
    Token* identifier = (Token*) malloc(sizeof(Token));

    if (identifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }

    identifier->codigo = TOKEN_OPER_IDENTIFICADOR;
    identifier->identificador = word;
    identifier->tokenEspecial = false;

    // cadastrarIdentifiersToClose();

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

        number *= 10;
        number += (int) (word[i] - '0');
    }

    Token* identifier = (Token*) malloc(sizeof(Token));

    if (identifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "getNumericValid não pôde alocar memória.");
    }

    identifier->codigo = TOKEN_OPER_NUMERO;
    identifier->identificador = word;
    identifier->tokenEspecial = false;

    // cadastrarNumbersToClose();
    
    return identifier;
}

// Tenta casar 'candidato' contra as definições de token, filtrando por tokenEspecial
// (false = palavra-chave/operador-palavra, true = símbolo) e por tamanho exato.
static Token* procurarDefinicao(const char* candidato, bool especial, int tamanho) {
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

static Token* getToken(char* word) {
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
    string currentIdentifier = (string) malloc(CAP_SIZE_IDENTIFIER);
    if (currentIdentifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial");
    }

    associarPonteirosParaErros(currentIdentifier, NULL);
    memset(currentIdentifier, '\0', CAP_SIZE_IDENTIFIER);

    char currentChar = 0;



    while (true) {
        currentChar = fgetc(currentFile);

        if (isspace(currentChar) || feof(currentFile)) {
            atualizarPosicao(currentChar);

            if (strlen(currentIdentifier) > 0) {
                storedToken = getToken(currentIdentifier);
                return;
            }

            if (feof(currentFile)) {
                storedToken = &tokenEof;
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
                if (foundSpecial == foundSpecialDuplo) {
                    atualizarPosicao(proximoChar);
                }
                else {
                    ungetc(proximoChar, currentFile);
                }

                atualizarPosicao(currentChar);

                storedToken = foundSpecial;
                return;
            }
        }

        // Encontrou Conteúdo Útil

        currentPosition.linha = filePosition.linha;
        currentPosition.coluna = filePosition.coluna;

        int tamanho = strlen(currentIdentifier);
        if (tamanho + 1 >= CAP_SIZE_IDENTIFIER) {
            sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Identificador Excede o Tamanho Máximo de Caracteres (36)");
        }

        currentIdentifier[tamanho] = currentChar;
        atualizarPosicao(currentChar);
    }
}

Token* getCurrentToken() {
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