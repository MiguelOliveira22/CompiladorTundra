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

    identifier->codigo = TOKEN_OPER_IDENTIFICADOR;
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

// Atualiza o rastreamento de posição (linha/coluna) para o caractere consumido.
// Mantive a mesma convenção de campos que já existia (linha zera e coluna incrementa
// a cada '\n'; linha incrementa nos demais casos).
static void atualizarPosicao(char c) {
    if (c == '\n') {
        filePosition.linha = 0;
        filePosition.coluna += 1;
    }
    else {
        filePosition.linha += 1;
    }
}

// Tenta casar 'candidato' contra as definições de token, filtrando por tokenEspecial
// (false = palavra-chave/operador-palavra, true = símbolo) e por tamanho exato.
static Token* casarDefinicao(const char* candidato, bool especial, int tamanho) {
    for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i++) {
        if (tokenDefinitions[i].tokenEspecial == especial &&
            strlen(tokenDefinitions[i].identificador) == tamanho &&
            strcmp(candidato, tokenDefinitions[i].identificador) == 0)
        {
            return &tokenDefinitions[i];
        }
    }
    return NULL;
}

void readNextToken(FILE* currentFile) {
    string currentIdentifier = (string) malloc(CAP_SIZE_IDENTIFIER);
    if (currentIdentifier == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "A Função de Coleta de Token Não Pôde Alocar Uma Variavel Essencial");
    }
    associarPonteirosParaErros(currentIdentifier, NULL);        // Alterar nulo depois eu acho
    memset(currentIdentifier, '\0', CAP_SIZE_IDENTIFIER);

    int c;

    // 1) Pula espaços em branco e comentários "(* ... *)".
    while (1) {
        c = fgetc(currentFile);

        if (c == EOF) {
            storedToken = &tokenEof;
            return;
        }

        if (isspace(c)) {
            atualizarPosicao((char) c);
            continue;
        }

        if (c == '(') {
            int lookahead = fgetc(currentFile);

            if (lookahead == '*') {
                atualizarPosicao('(');
                atualizarPosicao('*');

                int anterior = 0;
                int atual;
                while (1) {
                    atual = fgetc(currentFile);
                    if (atual == EOF) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Comentário Não Foi Fechado Antes do Fim do Arquivo");
                    }
                    atualizarPosicao((char) atual);
                    if (anterior == '*' && atual == ')') {
                        break;
                    }
                    anterior = atual;
                }
                continue; // volta a procurar o próximo token de verdade
            }

            if (lookahead != EOF) {
                ungetc(lookahead, currentFile);
            }
        }

        break; // 'c' é o primeiro caractere de um token de verdade
    }

    currentPosition.linha = filePosition.linha;
    currentPosition.coluna = filePosition.coluna;

    // 2) Identificador ou palavra-chave (and/or/not incluídos, pois não são "especiais").
    if (isalpha(c)) {
        int tamanho = 0;
        currentIdentifier[tamanho++] = (char) c;
        atualizarPosicao((char) c);

        int prox;
        while ((prox = fgetc(currentFile)) != EOF && (isalnum(prox) || prox == '_')) {
            if (tamanho + 1 >= CAP_SIZE_IDENTIFIER) {
                sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Identificador Excede o Tamanho Máximo de Caracteres (36)");
            }
            currentIdentifier[tamanho++] = (char) prox;
            atualizarPosicao((char) prox);
        }
        currentIdentifier[tamanho] = '\0';
        if (prox != EOF) {
            ungetc(prox, currentFile);
        }

        Token* palavraChave = casarDefinicao(currentIdentifier, false, tamanho);
        if (palavraChave != NULL) {
            storedToken = palavraChave;
            return;
        }

        Token* identificador = getIdentifierValid(currentIdentifier);
        if (identificador != NULL) {
            storedToken = identificador;
            return;
        }

        storedToken = &tokenInvalido;
        return;
    }

    // 3) Número.
    if (isdigit(c)) {
        int tamanho = 0;
        currentIdentifier[tamanho++] = (char) c;
        atualizarPosicao((char) c);

        int prox;
        while ((prox = fgetc(currentFile)) != EOF && isdigit(prox)) {
            if (tamanho + 1 >= CAP_SIZE_IDENTIFIER) {
                sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Identificador Excede o Tamanho Máximo de Caracteres (36)");
            }
            currentIdentifier[tamanho++] = (char) prox;
            atualizarPosicao((char) prox);
        }
        currentIdentifier[tamanho] = '\0';
        if (prox != EOF) {
            ungetc(prox, currentFile);
        }

        Token* numero = getNumericValid(currentIdentifier);
        if (numero != NULL) {
            storedToken = numero;
            return;
        }

        storedToken = &tokenInvalido;
        return;
    }

    // 4) Símbolo: tenta achar um de 2 caracteres primeiro (ex: ":=", "<="), senão de 1.
    char doisCaracteres[3] = { (char) c, '\0', '\0' };
    int segundo = fgetc(currentFile);

    if (segundo != EOF) {
        doisCaracteres[1] = (char) segundo;

        Token* simboloDuplo = casarDefinicao(doisCaracteres, true, 2);
        if (simboloDuplo != NULL) {
            atualizarPosicao((char) c);
            atualizarPosicao((char) segundo);
            storedToken = simboloDuplo;
            return;
        }

        ungetc(segundo, currentFile);
    }

    char umCaractere[2] = { (char) c, '\0' };
    Token* simboloUnico = casarDefinicao(umCaractere, true, 1);
    atualizarPosicao((char) c);

    if (simboloUnico != NULL) {
        storedToken = simboloUnico;
        return;
    }

    storedToken = &tokenInvalido;
}

Token* getCurrentToken() {
    return storedToken;
}

Token* getNextToken(FILE* currentFile){
    readNextToken(currentFile);
    return getCurrentToken();
}