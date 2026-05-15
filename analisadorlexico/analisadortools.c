#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "analisadortools.h"
#include "error.h"
#include "basics.h"

const i8 MAX_LENGTH_LEXICO = 36;
const token tokens[] = {
    { programa,        "program",   false, "programa"        },
    { rotulo,          "label",     false, "rotulo"          },
    { tipo,            "type",      false, "tipo"            },
    { variavel,        "var",       false, "variavel"        },
    { procedimento,    "procedure", false, "procedimento"    },
    { funcao,          "function",  false, "funcao"          },
    { inicio,          "begin",     false, "inicio"          },
    { fim,             "end",       false, "fim"             },
    { atribuicao,      ":=",        true,  "atribuicao"      },
    { se,              "if",        false, "se"              },
    { entao,           "then",      false, "entao"           },
    { senao,           "else",      false, "senao"           },
    { enquanto,        "while",     false, "enquanto"        },
    { faca,            "do",        false, "faca"            },
    { vapara,          "goto",      false, "vapara"          },

    { mais,            "+",         true,  "mais"            },
    { menos,           "-",         true,  "menos"           },
    { vezes,           "*",         true,  "vezes"           },
    { dividir,         "/",         true,  "dividir"         },

    { igual,           "=",         true,  "igual"           },
    { diferente,       "<>",        true,  "diferente"       },
    { menor,           "<",         true,  "menor"           },
    { menorouigual,    "<=",        true,  "menorouigual"    },
    { maior,           ">",         true,  "maior"           },
    { maiorouigual,    ">=",        true,  "maiorouigual"    },

    { e,               "and",       false, "e"               },
    { ou,              "or",        false, "ou"              },
    { nao,             "not",       false, "nao"             },

    { abreparenteses,  "(",         true,  "abreparenteses"  },
    { fechaparenteses, ")",         true,  "fechaparenteses" },
    { abrecolchetes,   "[",         true,  "abrecolchetes"   },
    { fechacolchetes,  "]",         true,  "fechacolchetes"  },
    { virgula,         ",",         true,  "virgula"         },
    { pontoevirgula,   ";",         true,  "pontoevirgula"   },
    { doispontos,      ":",         true,  "doispontos"      },
    { ponto,           ".",         true,  "ponto"           },
    
    { abrecomentario,  "(*",        true,  "abrecomentario"  },
    { fechacomentario, "*)",        true,  "fechacomentario" },
    
    { invalido,        "",          false, "invalido"        },
    { eof,             "",          false, "EOF"             }
};

int linha = 1;
int coluna = 0;

int linhaAtual = 1;
int colunaAtual = 0;

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
    compilaPrograma(file, escopo);
}

token analisarArquivo(FILE* file) {
    char palavra[MAX_LENGTH_LEXICO];
    memset(palavra, '\0', MAX_LENGTH_LEXICO);
    
    linha = linhaAtual;
    coluna = colunaAtual;

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
        sairErro(file, tokenInexperado, "Esperava-se um abre parenteses!");
    }
    
    while (tokenValue.codigoToken != fechaparenteses) {
        tokenValue = analisarArquivo(file);
        if (tokenValue.codigoToken != identificador) {
            sairErro(file, tokenInexperado, "Esperava-se um identificador!");
        }
        
        tokenValue = analisarArquivo(file);
        if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechaparenteses) {
            sairErro(file, tokenInexperado, "Esperava-se um virgula ou um fecha parenteses!");
        }
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != pontoevirgula) {
        sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula!");
    }
    
    compilaBloco(file, escopo + 1);
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != ponto) {
        sairErro(file, tokenInexperado, "Esperava-se um ponto final!");
    }
    
    tokenValue = analisarArquivo(file);
    if (tokenValue.codigoToken != eof) {
        sairErro(file, tokenInexperado, "Esperava-se fim de arquivo!");
    }
    
    sairErro(file, operacaoSucesso, "Programa sintaticamente correto!");
}

void compilaBloco(FILE* file, int escopo) {
    token tokenValue;
    
    while (true) {
        tokenValue = analisarArquivo(file);
        
        if (tokenValue.codigoToken == rotulo) {
            do {
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != numero) {
                    sairErro(file, tokenInexperado, "Esperava-se um número");
                }
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se virgula ou ponto e virgula");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.codigoToken == tipo) {
            do {
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != atribuicao) {
                    sairErro(file, tokenInexperado, "Esperava-se um simbolo de atribuicao");
                }
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou ponto e virgula");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.codigoToken == variavel) {
            do {
                do {
                    tokenValue = analisarArquivo(file);
                    if (tokenValue.codigoToken != identificador) {
                        sairErro(file, tokenInexperado, "Esperava-se um identificador");
                    }
                    
                    tokenValue = analisarArquivo(file);
                    if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                        sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                    }
                }
                while(tokenValue.codigoToken != doispontos);
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analisarArquivo(file);
                if (tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se um pontoevirgula");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
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
        
        sairErro(file, tokenInexperado, "OI");
    }
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
