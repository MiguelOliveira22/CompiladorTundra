#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "symboltable.h"
#include "analexico.h"
#include "tokenlexico.h"
#include "error.h"
#include "basics.h"

void compilaPrograma(FILE* file, int escopo);
void compilaBloco(FILE* file, int escopo);
void compilaParametrosFormais(FILE* file, int escopo);
void compilaComando(FILE* file, int escopo);
void compilaComandoSemRotulo(FILE* file, int escopo);
void compilaExpressao(FILE* file, int escopo);
void compilaExpressaoSimples(FILE* file, int escopo);
void compilaTermo(FILE* file, int escopo);
void compilaFator(FILE* file, int escopo);

void anasin(FILE* file) {
    compilaPrograma(file, 0);
}

void compilaPrograma(FILE* file, int escopo) {
    Token* tokenValue;
    

    tokenValue = getNextToken(file);
    if (tokenValue->codigo != TOKEN_KEYW_PROGRAMA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um PROGRAM!");
    }
    
    tokenValue = getNextToken(file);

    // TODO: Adicionar p/ symboltable
    if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um IDENTIFICADOR!");
    }
    
    tokenValue = getNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um abre parenteses!");
    }
    
    while (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
        tokenValue = getNextToken(file);
        // TODO: Adicionar p/ symboltable
        if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador!");
        }
        
        tokenValue = getNextToken(file);
        if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um virgula ou um fecha parenteses!");
        }
    }
    
    tokenValue = getNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula!");
    }
    
    tokenValue = getNextToken(file);
    compilaBloco(file, escopo);
    
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_SYMB_PONTO) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto final!");
    }
    
    tokenValue = getNextToken(file);
    if (tokenValue->codigo != TOKEN_OPER_EOF) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fim de arquivo!");
    }
    
    sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa sintaticamente correto!");
}

void compilaBloco(FILE* file, int escopo) {
    Token* tokenValue;

    while (true) {
        tokenValue = getCurrentToken();
        
        if (tokenValue->codigo == TOKEN_KEYW_ROTULO) {
            do {
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_NUMERO) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um número");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se virgula ou ponto e virgula");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA);
            
            getNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_TIPO) {
            tokenValue = getNextToken(file); // Pega o primeiro identificador
            
            do {
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_IGUAL) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um igual");
                }
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = getNextToken(file); // Sneakar o identificador
            }
            while(tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL) {   // Arrumar a virgula
            getNextToken(file);
            do {
                do {
                    tokenValue = getCurrentToken();
                    
                    if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                    }
                    
                    tokenValue = getNextToken(file);
                    if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                    }
                    
                    if (tokenValue->codigo == TOKEN_SYMB_VIRGULA) {
                        getNextToken(file);
                    }
                }
                while(tokenValue->codigo != TOKEN_SYMB_DOISPONTOS);
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = getNextToken(file); // Sneakar o identificador
            }
            while(tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_PROCEDIMENTO) {
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
            }
            
            getNextToken(file);
            compilaBloco(file, escopo + 1);
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            getNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_FUNCAO) {
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = getCurrentToken(); // Pega o doispontos
            if (tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
            }
            
            // Problema! Precisamos implementar tipos para a tabela de simbolos
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            getNextToken(file);
            compilaBloco(file, escopo + 1);
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            getNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_INICIO) { // Verificar aqui
            do {
                compilaComando(file, escopo);
                
                tokenValue = getCurrentToken(); // Verifica se encontrou end
            }
            while (tokenValue->codigo != TOKEN_KEYW_FIM);
            
            getNextToken(file); // Sneaky
            
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Bloco");
    }
}

void compilaParametrosFormais(FILE* file, int escopo) { // NEXT: analex(..., true);
    Token* tokenValue;
    
    tokenValue = getNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        return;
    }
    
    while (true) {
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL || tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) {
            if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL) {
                tokenValue = getNextToken(file);
            }
            
            do {
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
                
                if (tokenValue->codigo == TOKEN_SYMB_VIRGULA) {
                    tokenValue = getNextToken(file);
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_DOISPONTOS);
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_FUNCAO) {
            do {
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_DOISPONTOS);
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_PROCEDIMENTO) {
            do {
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA &&
                    tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && 
                    tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES)
                {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES);
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_FECHAPARENTESES) {
            getNextToken(file);
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se [VAR], [FUNC], [PROC] ou [ID]");
    }
}

void compilaComando(FILE* file, int escopo) { // NEXT: analex(..., false);
    Token* tokenValue;
    
    tokenValue = getNextToken(file);
    
    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {   // Comando Padrão e Adição de Rótulo
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
        }
        
        tokenValue = getNextToken(file);
    }
    
    compilaComandoSemRotulo(file, escopo);
}

void compilaComandoSemRotulo(FILE* file, int escopo) { // NEXT: analex(..., false);
    Token* tokenValue;
    
    tokenValue = getCurrentToken();
    
    if (tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) { // Atribuição, procedimento e função
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) { // Chamada Função
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = getCurrentToken();
                
                if (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES && tokenValue->codigo != TOKEN_SYMB_VIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou fechaparenteses");
                }
            }
            while (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES);
            
            getNextToken(file); // Sneak
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES || tokenValue->codigo == TOKEN_SYMB_ATRIBUICAO) {
            if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES) {
                do {
                    compilaExpressao(file, escopo);
                    
                    tokenValue = getCurrentToken();
                    
                    if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHACOLCHETES) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                    }
                }
                while(tokenValue->codigo != TOKEN_SYMB_FECHACOLCHETES);
                
                tokenValue = getNextToken(file);
            }
            
            if (tokenValue->codigo != TOKEN_SYMB_ATRIBUICAO) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se atribuicao");
            }
            
            compilaExpressao(file, escopo); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_VAPARA) {
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo != TOKEN_OPER_NUMERO) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se numero");
        }
        
        getNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_INICIO) {
        do {
            getNextToken(file);
            
            compilaComando(file, escopo + 1);
            
            tokenValue = getCurrentToken();
            
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_KEYW_FIM) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um fim ou pontoevirgula");
            }
        }
        while (tokenValue->codigo != TOKEN_KEYW_FIM);
        
        getNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_SE) {
        compilaExpressao(file, escopo);
        
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_KEYW_ENTAO) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se entao");
        }
        
        tokenValue = getNextToken(file);
        compilaComandoSemRotulo(file, escopo);
        
        tokenValue = getCurrentToken(); // Gets Skenay
        if (tokenValue->codigo != TOKEN_KEYW_SENAO) {
            return;
        }
        
        tokenValue = getNextToken(file);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_ENQUANTO) {
        compilaExpressao(file, escopo);
        
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_KEYW_FACA) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se faca");
        }
        
        getNextToken(file);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se identificador, vapara, se, enquanto ou inicio");
}

void compilaExpressao(FILE* file, int escopo) { // NEXT: analex(..., false);
    Token* tokenValue;
    
    compilaExpressaoSimples(file, escopo); 
    
    tokenValue = getCurrentToken(); // Gets sneaky
    if (
        tokenValue->codigo == TOKEN_SYMB_IGUAL ||
        tokenValue->codigo == TOKEN_SYMB_DIFERENTE ||
        tokenValue->codigo == TOKEN_SYMB_MAIORIGUAL ||
        tokenValue->codigo == TOKEN_SYMB_MENORIGUAL ||
        tokenValue->codigo == TOKEN_SYMB_MAIOR ||
        tokenValue->codigo == TOKEN_SYMB_MENOR
    ) {
        compilaExpressaoSimples(file, escopo); // Sneaky
    }
    
    return;
}

void compilaExpressaoSimples(FILE* file, int escopo) { // NEXT: analex(..., false);
    Token* tokenValue;
    
    tokenValue = getNextToken(file);
    
    if (tokenValue->codigo == TOKEN_SYMB_MAIS || tokenValue->codigo == TOKEN_SYMB_MENOS) {
        tokenValue = getNextToken(file);
    }
    
    compilaTermo(file, escopo);
    
    while (true) {
        tokenValue = getCurrentToken();
        
        if (
            tokenValue->codigo == TOKEN_SYMB_MAIS ||
            tokenValue->codigo == TOKEN_SYMB_MENOS ||
            tokenValue->codigo == TOKEN_SYMB_OU
        ) {
            getNextToken(file);
            compilaTermo(file, escopo);
        }
        else {
            return;
        }
    }
}

void compilaTermo(FILE* file, int escopo) { // NEXT: analex(..., false);
    Token* tokenValue;
    
    compilaFator(file, escopo);
    
    while (true) {
        tokenValue = getCurrentToken(); // Gets Sneaky
        
        if (
            tokenValue->codigo == TOKEN_SYMB_VEZES ||
            tokenValue->codigo == TOKEN_SYMB_DIVIDIR ||
            tokenValue->codigo == TOKEN_SYMB_E
        ) {
            getNextToken(file);
            compilaFator(file, escopo);
        }
        else {
            return;
        }
    }
}

void compilaFator(FILE* file, int escopo) { // NEXT: analex(..., false);
    Token* tokenValue;
    
    tokenValue = getCurrentToken();
    
    if (tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) {
        tokenValue = getNextToken(file); // Sneaky
        
        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = getCurrentToken(); // Gets Sneaky
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES);
            
            getNextToken(file); // Sneaky
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = getCurrentToken(); // Gets Senaky
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHACOLCHETES) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_FECHACOLCHETES);
            
            getNextToken(file); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {
        getNextToken(file); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
        compilaExpressao(file, escopo);
        
        tokenValue = getCurrentToken(); // Gets Sneaky
        if (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fechaparenteses");
        }
        
        getNextToken(file); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_SYMB_NAO) {
        getNextToken(file);
        compilaFator(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Fator");
}