#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "analisadortools.h"
#include "error.h"

token analisarArquivo(FILE* file);
token getToken(char* palavra);

token analex(FILE* file) {
    while (true) {
        token returnedToken = analisarArquivo(file);
        
        if (returnedToken.codigoToken == abrecomentario) {
            while(returnedToken.codigoToken != fechacomentario) {
                returnedToken = analisarArquivo(file);
            }
        }
        else {
            return returnedToken;
        }
    }
}

void anasin(FILE* file, int escopo) {
    
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
        
        if (feof(file) || isspace(palavra[currentTamanho])) {
            palavra[currentTamanho] = '\0';
            
            token currentToken = getToken(palavra);
            
            /*if (currentToken.numeroToken != null) {
                return currentToken;
            }*/
        }
        else {
            for (int i = 0; i < sizeof(tokens) / sizeof(token); i ++) {
                token currentToken = tokens[i];
                
                char lastChar[1];
                lastChar[0] = palavra[currentTamanho];
                
                if (strncmp(lastChar, currentToken.lexicoArquivo, 1) == 0 && currentToken.isEspecial) {
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
    
    /*if (currentTamanho <= 0) {
        return (token) { null, "", false, "NULL" };
    }*/
    
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

void compilaPrograma(FILE* file, int escopo)
{
    token tokenValue;
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != programa) {
        sairErro(file, tokenInexperado, "Esperava-se um PROGRAM!");
    }
    
    tokenValue = analisarArquivo(file);
    
    if (tokenValue.codigoToken != identificador) {
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != abreparenteses) {
        printf("Esperava-se um abre parenteses!");
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    while (tokenValue.codigoToken != fechaparenteses) {
        tokenValue = analisarArquivo(file);
        if (tokenValue.codigoToken != identificador) {
            printf("Esperava-se um identificador!");
            sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
        }
        
        tokenValue = analisarArquivo(file);
        if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechaparenteses) {
            printf("Esperava-se um virgula ou um fecha parenteses!");
            sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
        }
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != pontoevirgula) {
        printf("Esperava-se um ponto e virgula!");
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    compilaBloco(file, escopo + 1);
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != ponto) {
        printf("Esperava-se um ponto final!");
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != eof) {
        printf("Esperava-se fim de arquivo!");
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    sairErro(file, operacaoSucesso, "Programa sintaticamente correto!");
}

void compilaBloco(FILE* file, int escopo) {
    token tokenValue;
    
    while (true) {
        tokenValue = analisarArquivo(file);
        
        if (tokenValue.codigoToken == rotulo) {
            do {
                printf("", tokenValue.nomeCompilador);
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != numero) {
                    sairErro(8, file);
                }
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(9, file);
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.codigoToken == tipo) {
            continue;
        }
        
        if (tokenValue.codigoToken == variavel) {
            continue;
        }
        
        if (tokenValue.codigoToken == procedimento) {
            continue;
        }
        
        if (tokenValue.codigoToken == funcao) {
            continue;
        }
        
        if (tokenValue.codigoToken == inicio) {
            return;
        }
    }
    
    printf("Esperava-se %s", tokenValue.nomeCompilador);
    sairErro(6, file);
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
