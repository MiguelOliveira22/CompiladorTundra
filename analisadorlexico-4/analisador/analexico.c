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

static void atualizarPosicao(char currentChar) {
    if (currentChar == '\n') {
        filePosition.linha = 0;
        filePosition.coluna ++;
    }
    else {
        filePosition.linha ++;
    }
}

static Token* getIdentifierValid(char* word) {
    int wordSize = strlen(word);
    if (wordSize == 0) { return false; }
    
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

static void findNextToken(FILE* currentFile) {
    string currentIdentifier = (string) malloc(CAP_SIZE_IDENTIFIER);
    if (currentIdentifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial");
    }

    associarPonteirosParaErros(currentIdentifier, NULL);
    memset(currentIdentifier, '\0', CAP_SIZE_IDENTIFIER);

    Token* bufferToken;
    char anteriorChar;
    char currentChar = NULL;

    while (true) {
        anteriorChar = currentChar;
        currentChar = fgetc(currentFile);

        if (feof(currentFile)) {
            storedToken = &tokenEof;
            return;
        }

        if (isspace(currentChar)) {
            atualizarPosicao(currentChar);
            continue;
        }

        char simples[] = { currentIdentifier[strlen(currentIdentifier) - 1], '\0' };
        Token* foundSpecial = procurarDefinicao(simples, true, 1);
        Token* foundSpecialDuplo;

        if (isspace(anteriorChar) || foundSpecial != NULL) {
            if (!isspace(anteriorChar)) {
                // Lê mais um
                char duplo[] = { currentIdentifier[ - 2], currentIdentifier[ - 1], '\0' };
                foundSpecialDuplo = procurarDefinicao(duplo, true, 2);
                if (foundSpecialDuplo != NULL && strlen(currentIdentifier) > 2) {
                    // Devolve o segundo que pegou
                }
            }

            if (strlen(currentIdentifier) == 2 && foundSpecialDuplo != NULL) {
                return foundSpecialDuplo;
            }

            if (strlen(currentIdentifier) == 1 && foundSpecial != NULL) {
                return foundSpecial;
            }

            // Devolve o primeiro que pegou

            bufferToken = procurarDefinicao(currentIdentifier, false, strlen(currentIdentifier));
            if (bufferToken != NULL) {
                storedToken = bufferToken;
                return;
            }

            bufferToken = getIdentifierValid(currentIdentifier);
            if (bufferToken != NULL) {
                storedToken = bufferToken;
                return;
            }

            bufferToken = getNumericValid(currentIdentifier);
            if (bufferToken != NULL) {
                storedToken = bufferToken;
                return;
            }

            storedToken = &tokenInvalido;
            return;
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
        }

        return getCurrentToken();
    }
}