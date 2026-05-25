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

token analisarArquivo(FILE* file);
token getToken(char* palavra);

void compilaPrograma(FILE* file, int escopo);
void compilaBloco(FILE* file, int escopo);
void compilaParametrosFormais(FILE* file, int escopo);
void compilaComandos(FILE* file, int escopo);
char peekc(FILE* file);

bool isIdentifier(char* word);
bool isNumeric(char* word);
int getNumeric(char* word);

int linha = 1;
int coluna = 0;

int linhaProximo = 1;
int colunaProximo = 0;

int linhaAtual = 1;
int colunaAtual = 0;

token analex(FILE* file) {
    while (true) {
        token returnedToken = analisarArquivo(file);
        
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

void anasin(FILE* file) {
    compilaPrograma(file, 0);
}

char peekc(FILE* file) {
    char c = getc(file);
    ungetc(c, file);
    return c;
}

token analisarArquivo(FILE* file) {
    char palavra[MAX_LENGTH_LEXICO];
    memset(palavra, '\0', MAX_LENGTH_LEXICO);
    int i = 0;
    char c;

    while ((c = getc(file)) != EOF) {
        colunaAtual++;
        if (c == '\n') {
            linhaAtual++;
            colunaAtual = 0;
        }

        // Se encontrar espaços, delimitamos a palavra atual caso exista
        if (isspace(c)) {
            if (i > 0) {
                palavra[i] = '\0';
                return getToken(palavra);
            }
            continue;
        }

        char prox = peekc(file);

        // Tratamento prioritário de Operadores Compostos (2 caracteres): :=, <>, <=, >=, (*, *)
        if ((c == ':' && prox == '=') || (c == '<' && prox == '>') || 
            (c == '<' && prox == '=') || (c == '>' && prox == '=') ||
            (c == '(' && prox == '*') || (c == '*' && prox == ')')) {
            
            if (i > 0) {
                ungetc(c, file);
                palavra[i] = '\0';
                return getToken(palavra);
            }
            
            palavra[0] = c;
            palavra[1] = getc(file);
            colunaAtual++;
            palavra[2] = '\0';
            return getToken(palavra);
        }

        // Tratamento de Símbolos Especiais Isolados (1 caractere)
        if (strchr("()[],;:.+-*/=", c) != NULL) {
            if (i > 0) {
                ungetc(c, file); // Devolve o separador para leitura no próximo token
                palavra[i] = '\0';
                return getToken(palavra);
            }
            palavra[0] = c;
            palavra[1] = '\0';
            return getToken(palavra);
        }

        // Acumulação de caracteres comuns alfanuméricos
        if (i < MAX_LENGTH_LEXICO - 1) {
            palavra[i++] = c;
        } else {
            sairErro(file, excedeuMaxLengthLexico, "Excedeu o comprimento maximo do token.");
        }
    }

    if (i > 0) {
        palavra[i] = '\0';
        return getToken(palavra);
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
    
    tokenValue = analex(file);
    if (tokenValue.codigoToken != programa) {
        sairErro(file, tokenInexperado, "Esperava-se um PROGRAM!");
    }
    
    tokenValue = analex(file);
    
    if (tokenValue.codigoToken != identificador) {
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    tokenValue = analex(file);
    if (tokenValue.codigoToken != abreparenteses) {
        sairErro(file, tokenInexperado, "Esperava-se um abre parenteses!");
    }
    
    while (tokenValue.codigoToken != fechaparenteses) {
        tokenValue = analex(file);
        if (tokenValue.codigoToken != identificador) {
            sairErro(file, tokenInexperado, "Esperava-se um identificador!");
        }
        
        tokenValue = analex(file);
        if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechaparenteses) {
            sairErro(file, tokenInexperado, "Esperava-se um virgula ou um fecha parenteses!");
        }
    }
    
    tokenValue = analex(file);
    if (tokenValue.codigoToken != pontoevirgula) {
        sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula!");
    }
    
    compilaBloco(file, escopo + 1);
    
    tokenValue = analex(file);
    if (tokenValue.codigoToken != ponto) {
        sairErro(file, tokenInexperado, "Esperava-se um ponto final!");
    }
    
    tokenValue = analex(file);
    if (tokenValue.codigoToken != eof) {
        sairErro(file, tokenInexperado, "Esperava-se fim de arquivo!");
    }
    
    sairErro(file, operacaoSucesso, "Programa sintaticamente correto!");
}
void compilaBloco(FILE* file, int escopo) {
    token tokenValue;
    
    while (true) {
        tokenValue = analex(file);
        
        if (tokenValue.codigoToken == rotulo) {
            do {
                tokenValue = analex(file);
                if (tokenValue.codigoToken != numero) {
                    sairErro(file, tokenInexperado, "Esperava-se um número");
                }
                tokenValue = analex(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se virgula ou ponto e virgula");
                }
            } while(tokenValue.codigoToken != pontoevirgula);
            continue;
        }
        
        if (tokenValue.codigoToken == tipo) {
            do {
                tokenValue = analex(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                tokenValue = analex(file);
                if (tokenValue.codigoToken != atribuicao) {
                    sairErro(file, tokenInexperado, "Esperava-se um simbolo de atribuicao");
                }
                tokenValue = analex(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                tokenValue = analex(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou ponto e virgula");
                }
            } while(tokenValue.codigoToken != pontoevirgula);
            continue;
        }
        
        if (tokenValue.codigoToken == variavel) {
            do {
                do {
                    tokenValue = analex(file);
                    if (tokenValue.codigoToken != identificador) {
                        sairErro(file, tokenInexperado, "Esperava-se um identificador");
                    }
                    tokenValue = analex(file);
                    if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                        sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                    }
                } while(tokenValue.codigoToken != doispontos);
                
                tokenValue = analex(file);
                if (tokenValue.codigoToken != identificador && tokenValue.codigoToken != numero) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador de tipo");
                }
                
                tokenValue = analex(file);
                if (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != atribuicao) {
                    sairErro(file, tokenInexperado, "Esperava-se um pontoevirgula ou atribuicao");
                }
                
                // Trata atribuições diretas inline do parse.tundra
                if (tokenValue.codigoToken == atribuicao) {
                    do {
                        tokenValue = analex(file);
                    } while (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != eof && tokenValue.codigoToken != ponto);
                }
            } while(tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != ponto && tokenValue.codigoToken != eof);
            
            if (tokenValue.codigoToken == ponto || tokenValue.codigoToken == eof) {
                ungetc('.', file); // Devolve para o escopo pai validar
                return;
            }
            continue;
        }
        
        if (tokenValue.codigoToken == procedimento) {
            tokenValue = analex(file);
            if (tokenValue.codigoToken != identificador) sairErro(file, tokenInexperado, "Esperava-se um identificador");
            compilaParametrosFormais(file, escopo);
            tokenValue = analex(file);
            if (tokenValue.codigoToken != pontoevirgula) sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            compilaBloco(file, escopo + 1);
            tokenValue = analex(file);
            if (tokenValue.codigoToken != pontoevirgula) sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            continue;
        }
        
        if (tokenValue.codigoToken == funcao) {
            tokenValue = analex(file);
            if (tokenValue.codigoToken != identificador) sairErro(file, tokenInexperado, "Esperava-se um identificador");
            compilaParametrosFormais(file, escopo);
            tokenValue = analex(file);
            if (tokenValue.codigoToken != doispontos) sairErro(file, tokenInexperado, "Esperava-se um dois pontos");
            tokenValue = analex(file);
            if (tokenValue.codigoToken != identificador) sairErro(file, tokenInexperado, "Esperava-se um identificador");
            tokenValue = analex(file);
            if (tokenValue.codigoToken != pontoevirgula) sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            compilaBloco(file, escopo + 1);
            tokenValue = analex(file);
            if (tokenValue.codigoToken != pontoevirgula) sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            continue;
        }
        
        if (tokenValue.codigoToken == inicio) {
            compilaComandos(file, escopo);
            return;
        }
        
        if (tokenValue.codigoToken == ponto || tokenValue.codigoToken == eof) {
            ungetc('.', file);
            return;
        }
        
        sairErro(file, tokenInexperado, "Erro Sintatico: Token inesperado no bloco.");
    }
}
void compilaParametrosFormais(FILE* file, int escopo) {
    token tokenValue;
    
    tokenValue = analex(file);
    if (tokenValue.codigoToken != abreparenteses) {
        // Se não for '(', significa que os parâmetros são opcionais e foram omitidos.
        // Devolvemos o caractere lido para o arquivo para que a função chamadora possa consumi-lo.
        if (tokenValue.codigoToken == pontoevirgula) {
            ungetc(';', file);
        } else if (tokenValue.codigoToken == doispontos) {
            ungetc(':', file);
        } else {
            // Se for qualquer outra coisa que não seja ';' ou ':', aí sim é um erro sintático.
            sairErro(file, tokenInexperado, "Esperava-se um abre parenteses");
        }
        return; // Sai da função com sucesso, pois não há parâmetros para processar
    }
    
    while (true) {
        tokenValue = analex(file);
        
        if (tokenValue.codigoToken == variavel || tokenValue.codigoToken == identificador) {
            if (tokenValue.codigoToken != identificador) {
                tokenValue = analex(file);
            }
            
            do {
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue.codigoToken != doispontos);
            
            tokenValue = analex(file);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um [ID]");
            }
            
            tokenValue = analex(file);
            if (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fechaparenteses) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue.codigoToken == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue.codigoToken == funcao) {
            do {
                tokenValue = analex(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue.codigoToken != doispontos);
            
            tokenValue = analex(file);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um [ID]");
            }
            
            tokenValue = analex(file);
            if (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fechaparenteses) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue.codigoToken == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue.codigoToken == procedimento) {
            do {
                tokenValue = analex(file);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file);
                if (tokenValue.codigoToken != virgula &&
                    tokenValue.codigoToken != pontoevirgula && 
                    tokenValue.codigoToken != fechaparenteses)
                {
                    sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula ou um fecha parenteses");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fechaparenteses);
            
            if (tokenValue.codigoToken == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue.codigoToken == fechaparenteses) {
            return;
        }
        
        sairErro(file, tokenInexperado, "Esperava-se [VAR], [FUNC], [PROC] ou [ID]");
    }
}

void compilaComandos(FILE* file, int escopo) {
    token tokenValue;
    
    while (true) {
        tokenValue = analex(file);
        
        // Se encontrar 'end', o bloco atual de comandos acabou
        if (tokenValue.codigoToken == fim || tokenValue.codigoToken == eof) {
            return;
        }
        
        // Se encontrar sub-blocos 'begin' internos, processa recursivamente
        if (tokenValue.codigoToken == inicio) {
            compilaComandos(file, escopo);
            continue;
        }
        
        // Se encontrar um rótulo numérico (ex: 100:)
        if (tokenValue.codigoToken == numero) {
            tokenValue = analex(file); // Consome o ':'
            if (tokenValue.codigoToken != doispontos) {
                sairErro(file, tokenInexperado, "Erro Sintático: Esperava-se ':' após o rótulo.");
            }
            continue;
        }
        
        // Se for um identificador (read, write, p, x) ou palavra-chave (if, while, goto)
        if (tokenValue.codigoToken == identificador || tokenValue.codigoToken == se || 
            tokenValue.codigoToken == enquanto || tokenValue.codigoToken == vapara) {
            
            // Avança e consome os elementos do comando atual até achar ';' ou 'end'
            while (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fim && tokenValue.codigoToken != eof) {
                tokenValue = analex(file);
            }
            
            // Se parou no 'end', devolve os caracteres para o laço principal capturar no topo do while
            if (tokenValue.codigoToken == fim) {
                ungetc('d', file); ungetc('n', file); ungetc('e', file);
            }
            
            // Continuará o laço 'while (true)' para ler o próximo comando da linha
            continue;
        }
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
