#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "analisadortools.h"
#include "error.h"

// Main Function
int main(int argc, char* argv[])
{
    if (argc <= 0) { exit(1); }
    
    FILE* file;
    file = fopen(argv[1], "r");
    
    if (file == NULL) { exit(2); }
    
    compilaPrograma(file, 0);
}

// Helper Functions
void compilaPrograma(FILE* file, int escopo)
{
    token tokenValue;
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.numeroToken != programa) {
        printf("Esperava-se a palavra PROGRAM!");
        sairErro(5, file);
    }
    
    tokenValue = analisarArquivo(file);
    
    if (tokenValue.numeroToken != identificador) {
        printf("Esperava-se um identificador!");
        sairErro(5, file);
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.numeroToken != abreparenteses) {
        printf("Esperava-se um abre parenteses!");
        sairErro(5, file);
    }
    
    while (tokenValue.numeroToken != fechaparenteses) {
        tokenValue = analisarArquivo(file);
        if (tokenValue.numeroToken != identificador) {
            printf("Esperava-se um identificador!");
            sairErro(5, file);
        }
        
        tokenValue = analisarArquivo(file);
        if (tokenValue.numeroToken != virgula && tokenValue.numeroToken != fechaparenteses) {
            printf("Esperava-se um virgula ou um fecha parenteses!");
            sairErro(5, file);
        }
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.numeroToken != pontoevirgula) {
        printf("Esperava-se um ponto e virgula!");
        sairErro(5, file);
    }
    
    compilaBloco(file, escopo + 1);
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.numeroToken != ponto) {
        printf("Esperava-se um ponto final!");
        sairErro(5, file);
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.numeroToken != eof) {
        printf("Esperava-se fim de arquivo!");
        sairErro(5, file);
    }
    
    printf("Programa sintaticamente correto!");
    sairErro(0, file);
}

void compilaBloco(FILE* file, int escopo) {
    token tokenValue;
    
    while (true) {
        tokenValue = analisarArquivo(file);
        
        if (tokenValue.numeroToken == rotulo) {
            do {
                printf("", tokenValue.tokenValor);
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.numeroToken != numero) {
                    sairErro(8, file);
                }
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.numeroToken != virgula && tokenValue.numeroToken != pontoevirgula) {
                    sairErro(9, file);
                }
            }
            while(tokenValue.numeroToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.numeroToken == tipo) {
            continue;
        }
        
        if (tokenValue.numeroToken == variavel) {
            continue;
        }
        
        if (tokenValue.numeroToken == procedimento) {
            continue;
        }
        
        if (tokenValue.numeroToken == funcao) {
            continue;
        }
        
        if (tokenValue.numeroToken == inicio) {
            return;
        }
    }
    
    printf("Esperava-se %s", tokenValue.tokenValor);
    sairErro(6, file);
}

token analisarArquivo(FILE* file) {
    char palavra[MAX_LENGTH_LEXICO];
    memset(palavra, '\0', MAX_LENGTH_LEXICO);
    
    i8 currentTamanho;

    while (!feof(file)) {
        currentTamanho = strlen(palavra);
        
        if (currentTamanho >= MAX_LENGTH_LEXICO - 1) { sairErro(4, file); }
        
        palavra[currentTamanho] = getc(file);
        
        if (feof(file) || isspace(palavra[currentTamanho])) {
            palavra[currentTamanho] = '\0';
            
            token currentToken = getToken(palavra);
            
            if (currentToken.numeroToken != null) {
                return currentToken;
            }
        }
        else {
            for (int i = 0; i < sizeof(tokens) / sizeof(token); i ++) {
                token currentToken = tokens[i];
                
                char lastChar[1];
                lastChar[0] = palavra[currentTamanho];
                
                if (strncmp(lastChar, currentToken.lexico, 1) == 0 && currentToken.isBreak) {
                    if (currentTamanho > 0) {
                        ungetc(palavra[currentTamanho], file);
                        palavra[currentTamanho] = '\0';
                        
                        return getToken(palavra);
                    }
                    else {
                        palavra[currentTamanho + 1] = getc(file);
                        palavra[currentTamanho + 2] = '\0';
                        
                        token partialToken = getToken(palavra);
                        
                        if (partialToken.numeroToken == invalido) {
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
        
        if (strcmp(palavra, currentToken.lexico) == 0) {
            return currentToken;
        }
    }
    
    if (isIdentifier(palavra)) {
        char buffer[14 + MAX_LENGTH_LEXICO];
        sprintf(buffer, "identificador %s", palavra);
        
        return (token) { identificador, "", false, buffer };
    }
    else if (isNumeric(palavra)) {
        char buffer[7 + MAX_LENGTH_LEXICO];
        sprintf(buffer, "numero %d", getNumeric(palavra));
        
        return (token) { numero, "", false, buffer };
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

