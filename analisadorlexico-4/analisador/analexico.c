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

Token* getIdentifierValid(char* word) {
    int wordSize = strlen(word);
    
    if (wordSize == 0) {
        return false;
    }
    
    for (int i = 0; i < wordSize; i ++) {
        if (word[i] == '_' && i != 0) {
            continue;
        }
        
        if ((word[i] >= '0' && word[i] <= '9') && i != 0) {
            continue;
        }
        
        if (word[i] >= 'A' && word[i] <= 'Z') {
            continue;
        }
        
        if (word[i] >= 'a' && word[i] <= 'z') {
            continue;
        }
        
        return NULL;
    }
    
    Token* identifier = (Token*) malloc(sizeof(Token));

    if (identifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }

    identifier->codigo = TOKEN_OPER_NUMERO;
    identifier->identificador = word;
    identifier->tokenEspecial = false;

    return identifier;
}

Token* getNumericValid(char* word) {
    int wordSize = strlen(word);
    
    if (wordSize == 0) {
        return NULL;
    }
    
    for (int i = 0; i < wordSize; i ++) {
        if (word[i] < '0' || word[i] > '9') {
            return NULL;
        }
    }
    
    int number = 0;
    for (int i = 0; i < wordSize; i ++) {
        number *= 10;
        
        if (word[i] >= '0' && word[i] <= '9') {
            number += (int) (word[i] - '0');
        }
    }

    Token* identifier = (Token*) malloc(sizeof(Token));

    if (identifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }

    identifier->codigo = TOKEN_OPER_NUMERO;
    identifier->identificador = word;
    identifier->tokenEspecial = false;
    
    return identifier;
}

void readNextToken(FILE* currentFile) {
    currentPosition.linha = filePosition.linha;
    currentPosition.coluna = filePosition.coluna;

    string currentIdentifier = (string) malloc(CAP_SIZE_IDENTIFIER);
    associarPonteirosParaErros(currentIdentifier, NULL);        // Alterar nulo depois eu acho

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
    Token* foundToken = NULL;

    while (storedToken != &tokenEof)
    {
        currentCharacter = (char) fgetc(currentFile);

        if (currentCharacter == EOF || isspace(currentCharacter)) {
            for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i ++) {
                if (strcmp(currentIdentifier, tokenDefinitions[i].identificador) == 0) {
                    foundToken = &tokenDefinitions[i];
                }
            }

            if (foundToken != NULL) {
                storedToken = foundToken;
                return;
            }

            foundToken = getIdentifierValid(currentIdentifier);
            if (foundToken != NULL) {
                storedToken = foundToken;
                return;
            }

            foundToken = getNumericValid(currentIdentifier);
            if (foundToken != NULL) {
                storedToken = foundToken;
                return;
            }
            else {
                storedToken = &tokenInvalido;
                return;
            }
        }
        else {
            for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i++) {
                memset(currentIdentifier, '\0', sizeof(char) * CAP_SIZE_IDENTIFIER);
                /*
                fgets(currentSpecialToken, strlen(tokenDefinitions[i].identificador), currentFile);
                if (strcmp(currentSpecialToken, tokenDefinitions[i].identificador) == 0 && tokenDefinitions[i].tokenEspecial) {
                    if (strlen(tokenDefinitions[i].identificador) > currentSpecialSize) {
                */
                fgets(currentIdentifier, strlen(tokenDefinitions[i].identificador), currentFile);
                if (strcmp(currentIdentifier, tokenDefinitions[i].identificador) == 0 && tokenDefinitions[i].tokenEspecial) {
                    if (strlen(tokenDefinitions[i].identificador) > strlen(currentIdentifier)) {
                        foundToken = &tokenDefinitions[i];
                        //currentSpecial = &tokenDefinitions[i];
                        //currentSpecialSize = strlen(tokenDefinitions[i].identificador);
                    }
                }
            }

            if (foundToken != NULL) {
                storedToken = foundToken;
                return;
                //strcat(currentIdentifier, &currentCharacter);
            }
            else{
                storedToken = &tokenInvalido;
                return;
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

Token* getNextToken(FILE* currentFile){
    readNextToken(currentFile);
    return getCurrentToken();
}