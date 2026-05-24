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
void compilaComando(FILE* file, int escopo);
void compilaComandoSemRotulo(FILE* file, int escopo);
void compilaExpressao(FILE* file, int escopo);
void compilaExpressaoSimples(FILE* file, int escopo);
void compilaTermo(FILE* file, int escopo);
void compilaFator(FILE* file, int escopo);

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
        printf("%02d - %s\n", returnedToken.codigoToken, returnedToken.nomeCompilador);
        
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
            
            printf("%02d - %s\n", returnedToken.codigoToken, returnedToken.nomeCompilador);
            
            return returnedToken;
        }
    }
}

void anasin(FILE* file) {
    compilaPrograma(file, 0);
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

void compilaPrograma(FILE* file, int escopo)
{
    token tokenValue;
    
    tokenValue = analex(file, true);
    if (tokenValue.codigoToken != programa) {
        sairErro(file, tokenInexperado, "Esperava-se um PROGRAM!");
    }
    
    tokenValue = analex(file, true);
    
    if (tokenValue.codigoToken != identificador) {
        sairErro(file, tokenInexperado, "Esperava-se um IDENTIFICADOR!");
    }
    
    tokenValue = analex(file, true);
    if (tokenValue.codigoToken != abreparenteses) {
        sairErro(file, tokenInexperado, "Esperava-se um abre parenteses!");
    }
    
    while (tokenValue.codigoToken != fechaparenteses) {
        tokenValue = analex(file, true);
        if (tokenValue.codigoToken != identificador) {
            sairErro(file, tokenInexperado, "Esperava-se um identificador!");
        }
        
        tokenValue = analex(file, true);
        if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechaparenteses) {
            sairErro(file, tokenInexperado, "Esperava-se um virgula ou um fecha parenteses!");
        }
    }
    
    tokenValue = analex(file, true);
    if (tokenValue.codigoToken != pontoevirgula) {
        sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula!");
    }
    
    compilaBloco(file, escopo);
    
    tokenValue = analex(file, true);
    if (tokenValue.codigoToken != ponto) {
        sairErro(file, tokenInexperado, "Esperava-se um ponto final!");
    }
    
    tokenValue = analex(file, true);
    if (tokenValue.codigoToken != eof) {
        sairErro(file, tokenInexperado, "Esperava-se fim de arquivo!");
    }
    
    sairErro(file, operacaoSucesso, "Programa sintaticamente correto!");
}

void compilaBloco(FILE* file, int escopo) {
    token tokenValue;

    while (true) {
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken == rotulo) {
            do {
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != numero) {
                    sairErro(file, tokenInexperado, "Esperava-se um número");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se virgula ou ponto e virgula");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.codigoToken == tipo) {   // Arrumar a virgula
            do {
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != atribuicao) {
                    sairErro(file, tokenInexperado, "Esperava-se um simbolo de atribuicao");
                }
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou ponto e virgula");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.codigoToken == variavel) {   // Arrumar a virgula
            do {
                do {
                    tokenValue = analex(file, true);
                    if (tokenValue.codigoToken != identificador) {
                        sairErro(file, tokenInexperado, "Esperava-se um identificador");
                    }
                    
                    tokenValue = analex(file, true);
                    if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                        sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                    }
                }
                while(tokenValue.codigoToken != doispontos);
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se um pontoevirgula");
                }
            }
            while(tokenValue.codigoToken != pontoevirgula);
            
            continue;
        }
        
        if (tokenValue.codigoToken == procedimento) {
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um identificador");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != pontoevirgula) {
                compilaParametrosFormais(file, escopo);
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se um pontoevirgula");
                }
            }
            
            compilaBloco(file, escopo + 1);
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            }
            
            continue;
        }
        
        if (tokenValue.codigoToken == funcao) {
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um identificador");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != doispontos) {
                compilaParametrosFormais(file, escopo);
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != doispontos) {
                    sairErro(file, tokenInexperado, "Esperava-se um doispontos");
                }
            }
            
            // Problema! Precisamos implementar tipos para a tabela de simbolos
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um identificador");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            }
            
            compilaBloco(file, escopo + 1);
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            }
            
            continue;
        }
        
        if (tokenValue.codigoToken == inicio) {
            while (true) {
                compilaComando(file, escopo);
                
                tokenValue = analex(file, false);
                if (tokenValue.codigoToken == fim) {
                    return;
                }
                
                if (tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se fim ou pontoevirgula");
                }
            }
        }
        
        sairErro(file, tokenInexperado, "OI");
    }
}

void compilaParametrosFormais(FILE* file, int escopo) { // NEXT: analex(..., true);
    token tokenValue;
    
    tokenValue = analex(file, false);
    if (tokenValue.codigoToken != abreparenteses) {
        sairErro(file, tokenInexperado, "Esperava-se um abre parenteses");
    }
    
    while (true) {
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken == variavel || tokenValue.codigoToken == identificador) {
            if (tokenValue.codigoToken != identificador) {
                tokenValue = analex(file, true);
            }
            
            do {
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue.codigoToken != doispontos);
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um [ID]");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fechaparenteses) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue.codigoToken == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue.codigoToken == funcao) {
            do {
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue.codigoToken != doispontos);
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um [ID]");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fechaparenteses) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue.codigoToken == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue.codigoToken == procedimento) {
            do {
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file, true);
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

void compilaComando(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = analex(file, true);
    
    if (tokenValue.codigoToken == numero) {   // Comando Padrão e Adição de Rótulo
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken != doispontos) {
            sairErro(file, tokenInexperado, "Esperava-se um doispontos");
        }
        
        tokenValue = analex(file, true);
    }
    
    compilaComandoSemRotulo(file, escopo);
}

void compilaComandoSemRotulo(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = analex(file, false);
    
    if (tokenValue.codigoToken == identificador) { // Atribuição, procedimento e função
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken == abreparenteses) { // Chamada Função
            while (true) {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false);
                
                if (tokenValue.codigoToken == fechaparenteses) {
                    analex(file, true);
                    return;
                }
                
                if (tokenValue.codigoToken != virgula) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula");
                }
            }
        }
        
        if (tokenValue.codigoToken == abrecolchetes || tokenValue.codigoToken == atribuicao) {
            if (tokenValue.codigoToken == abrecolchetes) {
                do {
                    compilaExpressao(file, escopo);
                    
                    tokenValue = analex(file, false);
                    
                    if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechacolchetes) {
                        sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um fechacolchetes");
                    }
                }
                while(tokenValue.codigoToken != fechacolchetes);
                
                tokenValue = analex(file, true);
            }
            
            if (tokenValue.codigoToken != atribuicao) {
                sairErro(file, tokenInexperado, "Esperava-se atribuicao");
            }
            
            compilaExpressao(file, escopo);
        }
        
        return;
    }
    
    if (tokenValue.codigoToken == vapara) {
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken != numero) {
            sairErro(file, tokenInexperado, "Esperava-se numero");
        }
        
        analex(file, true);
        return;
    }
    
    if (tokenValue.codigoToken == inicio) {
        while (true) {
            compilaComando(file, escopo + 1);
            
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken == fim) {
                analex(file, true);
                return;
            }
            
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            }
        }
    }
    
    if (tokenValue.codigoToken == se) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken != entao) {
            sairErro(file, tokenInexperado, "Esperava-se entao");
        }
        
        tokenValue = analex(file, true);
        
        compilaComandoSemRotulo(file, escopo);
        
        tokenValue = analex(file, false);
        
        analex(file, true);
        
        if (tokenValue.codigoToken != senao) {
            return;
        }
        
        compilaComandoSemRotulo(file, escopo);
        
        return;
    }
    
    if (tokenValue.codigoToken == enquanto) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false);
        
        if (tokenValue.codigoToken != faca) {
            sairErro(file, tokenInexperado, "Esperava-se faca");
        }
        
        compilaComandoSemRotulo(file, escopo);
        
        return;
    }
    
    sairErro(file, tokenInexperado, "Esperava-se identificador, vapara, se, enquanto ou inicio");
}

void compilaExpressao(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    compilaExpressaoSimples(file, escopo);
    
    tokenValue = analex(file, true);
    
    if (
        tokenValue.codigoToken == igual ||
        tokenValue.codigoToken == diferente ||
        tokenValue.codigoToken == maiorouigual ||
        tokenValue.codigoToken == menorouigual ||
        tokenValue.codigoToken == maior ||
        tokenValue.codigoToken == menor
    ) {
        compilaExpressaoSimples(file, escopo);
        analex(file, true);
    }
    
    return;
}

void compilaExpressaoSimples(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = analex(file, true);
    
    if (tokenValue.codigoToken == mais || tokenValue.codigoToken == menos) {
        tokenValue = analex(file, true);
    }
    
    compilaTermo(file, escopo);
    
    while (true) {
        tokenValue = analex(file, false);
        
        if (
            tokenValue.codigoToken == mais ||
            tokenValue.codigoToken == menos ||
            tokenValue.codigoToken == ou
        ) {
            tokenValue = analex(file, true);
            
            compilaTermo(file, escopo);
        }
        else {
            return;
        }
    }
}

void compilaTermo(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    compilaFator(file, escopo);
    
    while (true) {
        tokenValue = analex(file, false);
        
        if (
            tokenValue.codigoToken == vezes ||
            tokenValue.codigoToken == dividir ||
            tokenValue.codigoToken == e
        ) {
            tokenValue = analex(file, true);
            
            compilaFator(file, escopo);
        }
        else {
            return;
        }
    }
}

void compilaFator(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = analex(file, false);
    
    if (tokenValue.codigoToken == identificador) {
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken == abreparenteses) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false);
                
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechaparenteses) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue.codigoToken == fechaparenteses);
            
            analex(file, true);
            return;
        }
        
        if (tokenValue.codigoToken == abrecolchetes) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false);
                
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechacolchetes) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue.codigoToken == fechacolchetes);
            
            analex(file, true);
            return;
        }
        
        return;
    }
    
    if (tokenValue.codigoToken == numero) {
        analex(file, true);
        
        return;
    }
    
    if (tokenValue.codigoToken == abreparenteses) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false);
        
        if (tokenValue.codigoToken != fechaparenteses) {
            sairErro(file, tokenInexperado, "Esperava-se fechaparenteses");
        }
        
        analex(file, true);
        
        return;
    }
    
    if (tokenValue.codigoToken == nao) {
        compilaFator(file, escopo);
        
        return;
    }
    
    sairErro(file, tokenInexperado, "");
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
