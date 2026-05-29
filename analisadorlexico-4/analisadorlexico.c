#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "analisadortools.h"
#include "error.h"
#include "basics.h"

token analisarArquivo(FILE* file);
token getToken(char* palavra);

bool isIdentifier(char* word);
bool isNumeric(char* word);
int getNumeric(char* word);

int linha = 1;
int coluna = 0;

int linhaProximo = 1;
int colunaProximo = 0;

int linhaAtual = 1;
int colunaAtual = 0;

token returnedToken;

token analex(FILE* file, bool updateToken) {
    if (!updateToken) {
        return returnedToken;
    }
    
    while (true) {
        returnedToken = analisarArquivo(file);
        
        if (returnedToken.codigoToken == abrecomentario) {
            while(returnedToken.codigoToken != fechacomentario) {
                returnedToken = analisarArquivo(file);
                
                if (returnedToken.codigoToken == eof) {
                    sairErro(file, naoFechouComentario, "Não fechou comentário");
                }
            }
        }
        else {
            linha = linhaProximo;
            coluna = colunaProximo;
            
            linhaProximo = linhaAtual;
            colunaProximo = colunaAtual;
            
            return returnedToken;
        }
    }
}

token analisarArquivo(FILE* file) {
    char palavra[MAX_LENGTH_LEXICO];
    memset(palavra, '\0', MAX_LENGTH_LEXICO);

    while (!feof(file)) {
        i8 currentTamanho = strlen(palavra);
        
        if (currentTamanho >= MAX_LENGTH_LEXICO - 1) {
            sairErro(file, excedeuMaxLengthLexico, "Excedeu o comprimento máximo para um único token (36 caracteres).");
        }
        
        palavra[currentTamanho] = getc(file);
        
        colunaAtual ++;
        
        if (palavra[currentTamanho] == '\n') {
            linhaAtual ++;
            colunaAtual = 0;
        }
        
        if (feof(file) || isspace(palavra[currentTamanho])) {
            palavra[currentTamanho] = '\0';
            
            token currentToken = getToken(palavra);
            
            if (currentToken.codigoToken != null) {
                return currentToken;
            }
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
            }
        }
    }
    
    return (token) { eof, "", false, "EOF" };
}

token getToken(char* palavra) {
    i8 currentTamanho = strlen(palavra);
    
    if (currentTamanho <= 0) {
        return (token) { null, "", false, "NULL" };
    }
    
    for (int i = 0; i < sizeof(tokens) / sizeof(token); i ++) {
        token currentToken = tokens[i];
        
        if (strcmp(palavra, currentToken.lexicoArquivo) == 0) {
            return currentToken;
        }
    }
    
    if (isIdentifier(palavra)) {
        return (token) { identificador, "", false, palavra };
    }
    else if (isNumeric(palavra)) {
        return (token) { numero, "", false, palavra };
    }
    
    return (token) { invalido, "", false, "invalido" };
}

bool isIdentifier(char* word) {
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
        
        return false;
    }
    
    return true;
}

bool isNumeric(char* word) {
    int wordSize = strlen(word);
    
    if (wordSize == 0) {
        return false;
    }
    
    for (int i = 0; i < wordSize; i ++) {
        if (word[i] < '0' || word[i] > '9') {
            return false;
        }
    }
    
    return true;
}

int getNumeric(char* word) {
    int wordSize = strlen(word);
    
    int number = 0;
    for (int i = 0; i < wordSize; i ++) {
        number *= 10;
        
        if (word[i] >= '0' && word[i] <= '9') {
            number += (int) (word[i] - '0');
        }
    }
    
    return number;
}
