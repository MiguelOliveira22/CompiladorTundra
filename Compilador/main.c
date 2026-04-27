#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "analisadorlexico.h"

// Constants - Valores Arbitrarios e Tals
const i8 MAX_LENGTH_LEXICO = 26;

const token tokens[] = {
    { "program",   false, "programa"        },
    { "label",     false, "rotulo"          },
    { "type",      false, "tipo"            },
    { "var",       false, "variavel"        },
    { "procedure", false, "procedimento"    },
    { "function",  false, "funcao"          },
    { "begin",     false, "inicio"          },
    { "end",       false, "fim"             },
    { ":=",        true,  "atribuicao"      },
    { "if",        false, "se"              },
    { "then",      false, "entao"           },
    { "else",      false, "senao"           },
    { "while",     false, "enquanto"        },
    { "do",        false, "faca"            },
    { "goto",      false, "vapara"          },

    { "+",         true,  "mais"            },
    { "-",         true,  "menos"           },
    { "*",         true,  "vezes"           },
    { "/",         true,  "dividir"         },

    { "=",         true,  "igual"           },
    { "<>",        true,  "diferente"       },
    { "<",         true,  "menor"           },
    { "<=",        true,  "menorouigual"    },
    { ">",         true,  "maior"           },
    { ">=",        true,  "maiorouigual"    },

    { "and",       false, "e"               },
    { "or",        false, "ou"              },
    { "not",       false, "nao"             },

    { "(",         true,  "abreparenteses"  },
    { ")",         true,  "fechaparenteses" },
    { "[",         true,  "abrecolchetes"   },
    { "]",         true,  "fechacolchetes"  },
    { ",",         true,  "virgula"         },
    { ";",         true,  "pontoevirgula"   },
    { ":",         true,  "doispontos"      },
    { ".",         true,  "ponto"           }
};

// Main Function
int main(int argc, char* argv[])
{
    if (argc <= 0) { exit(1); }
    
    FILE* file;
    file = fopen(argv[1], "r");
    
    if (file == NULL) { exit(2); }
    
    token currentToken;
    do {
        currentToken = analisarArquivo(file);
        printf("%s\n", currentToken.tokenValor);
    }
    while (strcmp(currentToken.tokenValor, "EOF") != 0);
    
    fclose(file);
    
    exit(0);
    
    //compilaPrograma(file);
}

// Helper Functions
void compilaPrograma(FILE* file)
{
    token tokenValue;
    
    tokenValue = analisarArquivo(file);
    if (strcmp(tokenValue.tokenValor, "programa") != 0) {
        printf("Esperava-se a palavra PROGRAM!");
        sairErro(5, file);
    }
    
    tokenValue = analisarArquivo(file);
    
    if (strcmp(tokenValue.tokenValor, "identificador") != 0) {
        printf("Esperava-se um identificador!");
        sairErro(5, file);
    }
    
    tokenValue = analisarArquivo(file);
    if (strcmp(tokenValue.tokenValor, "abreparenteses") != 0) {
        printf("Esperava-se um abre parenteses!");
        sairErro(5, file);
    }
    
    while (strcmp(tokenValue.tokenValor, "fechaparenteses") != 0) {
        tokenValue = analisarArquivo(file);
        if (strcmp(tokenValue.tokenValor, "identificador") != 0) {
            printf("Esperava-se um identificador!");
            sairErro(5, file);
        }
        
        tokenValue = analisarArquivo(file);
        if (strcmp(tokenValue.tokenValor, "virgula") != 0 && strcmp(tokenValue.tokenValor, "fechaparenteses") != 0) {
            printf("Esperava-se um virgula ou um fecha parenteses!");
            sairErro(5, file);
        }
    }
    
    tokenValue = analisarArquivo(file);
    if (strcmp(tokenValue.tokenValor, "pontoevirgula") != 0) {
        printf("Esperava-se um ponto e virgula!");
        sairErro(5, file);
    }
    
    // compilaBloco();
    
    tokenValue = analisarArquivo(file);
    if (strcmp(tokenValue.tokenValor, "ponto") != 0) {
        printf("Esperava-se um ponto final!");
        sairErro(5, file);
    }
    
    tokenValue = analisarArquivo(file);
    if (strcmp(tokenValue.tokenValor, "EOF") != 0) {
        printf("Esperava-se fim de arquivo!");
        sairErro(5, file);
    }
    
    printf("Programa sintaticamente correto!");
    sairErro(0, file);
}

void sairErro(int codigo, FILE* file) {
    fclose(file);
    exit(codigo);
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
            
            if (strcmp(currentToken.tokenValor, "NULL") != 0) {
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
                        
                        if (strcmp(partialToken.tokenValor, "invalido") == 0) {
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
    
    return (token) { "", false, "EOF" };
}

token getToken(char* palavra) {
    i8 currentTamanho = strlen(palavra);
    
    if (currentTamanho <= 0) {
        return (token) { "", false, "NULL" };
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
        
        return (token) { "", false, buffer };
    }
    else if (isNumeric(palavra)) {
        char buffer[7 + MAX_LENGTH_LEXICO];
        sprintf(buffer, "numero %d", getNumeric(palavra));
        
        return (token) { "", false, buffer };
    }
    
    return (token) { "", false, "invalido" };
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
            if (i == 0 && (word[i] == '-' || word[i] == '+')) {
                continue;
            }
            
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
    
    if (word[0] == '-') {
        number = -number;
    }
    
    return number;
}
