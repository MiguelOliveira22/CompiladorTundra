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
    
    tokenValue = readCurrentToken();
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
        tokenValue = readCurrentToken();
        
        if (tokenValue->codigo == TOKEN_KEYW_ROTULO) {
            do {
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != numero) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um número");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != virgula && tokenValue->codigo != pontoevirgula) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se virgula ou ponto e virgula");
                }
            }
            while(tokenValue->codigo != pontoevirgula);
            
            getNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == tipo) {
            tokenValue = getNextToken(file); // Pega o primeiro identificador
            
            do {
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != igual) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um igual");
                }
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != pontoevirgula) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = getNextToken(file); // Sneakar o identificador
            }
            while(tokenValue->codigo == identificador);
            
            continue;
        }
        
        if (tokenValue->codigo == variavel) {   // Arrumar a virgula
            getNextToken(file);
            do {
                do {
                    tokenValue = readCurrentToken();
                    
                    if (tokenValue->codigo != identificador) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                    }
                    
                    tokenValue = getNextToken(file);
                    if (tokenValue->codigo != virgula && tokenValue->codigo != doispontos) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                    }
                    
                    if (tokenValue->codigo == virgula) {
                        getNextToken(file);
                    }
                }
                while(tokenValue->codigo != doispontos);
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != pontoevirgula) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = getNextToken(file); // Sneakar o identificador
            }
            while(tokenValue->codigo == identificador);
            
            continue;
        }
        
        if (tokenValue->codigo == procedimento) {
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = readCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
            }
            
            getNextToken(file);
            compilaBloco(file, escopo + 1);
            
            tokenValue = readCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            getNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == funcao) {
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = readCurrentToken(); // Pega o doispontos
            if (tokenValue->codigo != doispontos) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
            }
            
            // Problema! Precisamos implementar tipos para a tabela de simbolos
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            getNextToken(file);
            compilaBloco(file, escopo + 1);
            
            tokenValue = readCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            getNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == inicio) { // Verificar aqui
            do {
                compilaComando(file, escopo);
                
                tokenValue = readCurrentToken(); // Verifica se encontrou end
            }
            while (tokenValue->codigo != fim);
            
            getNextToken(file); // Sneaky
            
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Bloco");
    }
}

void compilaParametrosFormais(FILE* file, int escopo) { // NEXT: analex(..., true);
    token tokenValue;
    
    tokenValue = getNextToken(file);
    if (tokenValue->codigo != abreparenteses) {
        return;
    }
    
    while (true) {
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo == variavel || tokenValue->codigo == identificador) {
            if (tokenValue->codigo == variavel) {
                tokenValue = getNextToken(file);
            }
            
            do {
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != virgula && tokenValue->codigo != doispontos) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
                
                if (tokenValue->codigo == virgula) {
                    tokenValue = getNextToken(file);
                }
            }
            while(tokenValue->codigo != doispontos);
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != pontoevirgula && tokenValue->codigo != fechaparenteses) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue->codigo == funcao) {
            do {
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != virgula && tokenValue->codigo != doispontos) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue->codigo != doispontos);
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            tokenValue = getNextToken(file);
            if (tokenValue->codigo != pontoevirgula && tokenValue->codigo != fechaparenteses) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue->codigo == procedimento) {
            do {
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = getNextToken(file);
                if (tokenValue->codigo != virgula &&
                    tokenValue->codigo != pontoevirgula && 
                    tokenValue->codigo != fechaparenteses)
                {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
                }
            }
            while(tokenValue->codigo != pontoevirgula && tokenValue->codigo != fechaparenteses);
            
            if (tokenValue->codigo == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue->codigo == fechaparenteses) {
            getNextToken(file);
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se [VAR], [FUNC], [PROC] ou [ID]");
    }
}

void compilaComando(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = getNextToken(file);
    
    if (tokenValue->codigo == numero) {   // Comando Padrão e Adição de Rótulo
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo != doispontos) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
        }
        
        tokenValue = getNextToken(file);
    }
    
    compilaComandoSemRotulo(file, escopo);
}

void compilaComandoSemRotulo(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = readCurrentToken();
    
    if (tokenValue->codigo == identificador) { // Atribuição, procedimento e função
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo == abreparenteses) { // Chamada Função
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = readCurrentToken();
                
                if (tokenValue->codigo != fechaparenteses && tokenValue->codigo != virgula) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou fechaparenteses");
                }
            }
            while (tokenValue->codigo != fechaparenteses);
            
            getNextToken(file); // Sneak
        }
        
        if (tokenValue->codigo == abrecolchetes || tokenValue->codigo == atribuicao) {
            if (tokenValue->codigo == abrecolchetes) {
                do {
                    compilaExpressao(file, escopo);
                    
                    tokenValue = readCurrentToken();
                    
                    if (tokenValue->codigo != virgula && tokenValue->codigo != fechacolchetes) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                    }
                }
                while(tokenValue->codigo != fechacolchetes);
                
                tokenValue = getNextToken(file);
            }
            
            if (tokenValue->codigo != atribuicao) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se atribuicao");
            }
            
            compilaExpressao(file, escopo); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == vapara) {
        tokenValue = getNextToken(file);
        
        if (tokenValue->codigo != numero) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se numero");
        }
        
        getNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == inicio) {
        do {
            getNextToken(file);
            
            compilaComando(file, escopo + 1);
            
            tokenValue = readCurrentToken();
            
            if (tokenValue->codigo != pontoevirgula && tokenValue->codigo != fim) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um fim ou pontoevirgula");
            }
        }
        while (tokenValue->codigo != fim);
        
        getNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == se) {
        compilaExpressao(file, escopo);
        
        tokenValue = readCurrentToken();
        if (tokenValue->codigo != entao) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se entao");
        }
        
        tokenValue = getNextToken(file);
        compilaComandoSemRotulo(file, escopo);
        
        tokenValue = readCurrentToken(); // Gets Skenay
        if (tokenValue->codigo != senao) {
            return;
        }
        
        tokenValue = getNextToken(file);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == enquanto) {
        compilaExpressao(file, escopo);
        
        tokenValue = readCurrentToken();
        if (tokenValue->codigo != faca) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se faca");
        }
        
        getNextToken(file);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se identificador, vapara, se, enquanto ou inicio");
}

void compilaExpressao(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    compilaExpressaoSimples(file, escopo); 
    
    tokenValue = readCurrentToken(); // Gets sneaky
    if (
        tokenValue->codigo == igual ||
        tokenValue->codigo == diferente ||
        tokenValue->codigo == maiorouigual ||
        tokenValue->codigo == menorouigual ||
        tokenValue->codigo == maior ||
        tokenValue->codigo == menor
    ) {
        compilaExpressaoSimples(file, escopo); // Sneaky
    }
    
    return;
}

void compilaExpressaoSimples(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = getNextToken(file);
    
    if (tokenValue->codigo == mais || tokenValue->codigo == menos) {
        tokenValue = getNextToken(file);
    }
    
    compilaTermo(file, escopo);
    
    while (true) {
        tokenValue = readCurrentToken();
        
        if (
            tokenValue->codigo == mais ||
            tokenValue->codigo == menos ||
            tokenValue->codigo == ou
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
    token tokenValue;
    
    compilaFator(file, escopo);
    
    while (true) {
        tokenValue = readCurrentToken(); // Gets Sneaky
        
        if (
            tokenValue->codigo == vezes ||
            tokenValue->codigo == dividir ||
            tokenValue->codigo == e
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
    token tokenValue;
    
    tokenValue = readCurrentToken();
    
    if (tokenValue->codigo == identificador) {
        tokenValue = getNextToken(file); // Sneaky
        
        if (tokenValue->codigo == abreparenteses) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = readCurrentToken(); // Gets Sneaky
                if (tokenValue->codigo != virgula && tokenValue->codigo != fechaparenteses) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != fechaparenteses);
            
            getNextToken(file); // Sneaky
        }
        
        if (tokenValue->codigo == abrecolchetes) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = readCurrentToken(); // Gets Senaky
                if (tokenValue->codigo != virgula && tokenValue->codigo != fechacolchetes) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != fechacolchetes);
            
            getNextToken(file); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == numero) {
        getNextToken(file); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == abreparenteses) {
        compilaExpressao(file, escopo);
        
        tokenValue = readCurrentToken(); // Gets Sneaky
        if (tokenValue->codigo != fechaparenteses) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fechaparenteses");
        }
        
        getNextToken(file); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == nao) {
        getNextToken(file);
        compilaFator(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Fator");
}