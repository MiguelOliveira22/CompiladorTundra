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
    
    readNextToken(file);
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_KEYW_PROGRAMA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um PROGRAM!", file);
    }
    
    readNextToken(file);
    tokenValue = getCurrentToken();

    // TODO: Adicionar p/ symboltable
    if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um IDENTIFICADOR!", file);
    }
    
    readNextToken(file);
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um abre parenteses!", file);
    }
    
    while (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
        readNextToken(file);
        tokenValue = getCurrentToken();
        // TODO: Adicionar p/ symboltable
        if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador!", file);
        }
        
        readNextToken(file);
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um virgula ou um fecha parenteses!", file);
        }
    }
    
    readNextToken(file);
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula!", file);
    }
    
    readNextToken(file);
    tokenValue = getCurrentToken();
    compilaBloco(file, escopo);
    
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_SYMB_PONTO) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto final!", file);
    }
    
    readNextToken(file);
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_OPER_EOF) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fim de arquivo!", file);
    }
    
    sairErroTerminal(file, ERROR_OK_OPERATION_SUCCESS, "Programa sintaticamente correto!");
}

void compilaBloco(FILE* file, int escopo) {
    Token* tokenValue;

    while (true) {
        tokenValue = getCurrentToken();
        
        if (tokenValue->codigo == TOKEN_KEYW_ROTULO) {
            do {
                tokenValue = analex(file, true);
                if (tokenValue->codigo != numero) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um número");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != virgula && tokenValue->codigo != pontoevirgula) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se virgula ou ponto e virgula");
                }
            }
            while(tokenValue->codigo != pontoevirgula);
            
            analex(file, true);
            
            continue;
        }
        
        if (tokenValue->codigo == tipo) {
            tokenValue = analex(file, true); // Pega o primeiro identificador
            
            do {
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != igual) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um igual");
                }
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analex(file, true);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != pontoevirgula) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = analex(file, true); // Sneakar o identificador
            }
            while(tokenValue->codigo == identificador);
            
            continue;
        }
        
        if (tokenValue->codigo == variavel) {   // Arrumar a virgula
            analex(file, true);
            do {
                do {
                    tokenValue = getCurrentToken();
                    
                    if (tokenValue->codigo != identificador) {
                        sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                    }
                    
                    tokenValue = analex(file, true);
                    if (tokenValue->codigo != virgula && tokenValue->codigo != doispontos) {
                        sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                    }
                    
                    if (tokenValue->codigo == virgula) {
                        analex(file, true);
                    }
                }
                while(tokenValue->codigo != doispontos);
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analex(file, true);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != pontoevirgula) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = analex(file, true); // Sneakar o identificador
            }
            while(tokenValue->codigo == identificador);
            
            continue;
        }
        
        if (tokenValue->codigo == procedimento) {
            tokenValue = analex(file, true);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = analex(file, false); // Pega o Sneaky
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
            }
            
            analex(file, true);
            compilaBloco(file, escopo + 1);
            
            tokenValue = analex(file, false); // Pega o Sneaky
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            analex(file, true);
            
            continue;
        }
        
        if (tokenValue->codigo == funcao) {
            tokenValue = analex(file, true);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = analex(file, false); // Pega o doispontos
            if (tokenValue->codigo != doispontos) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
            }
            
            // Problema! Precisamos implementar tipos para a tabela de simbolos
            tokenValue = analex(file, true);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            analex(file, true);
            compilaBloco(file, escopo + 1);
            
            tokenValue = analex(file, false); // Pega o Sneaky
            if (tokenValue->codigo != pontoevirgula) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            analex(file, true);
            
            continue;
        }
        
        if (tokenValue->codigo == inicio) { // Verificar aqui
            do {
                compilaComando(file, escopo);
                
                tokenValue = analex(file, false); // Verifica se encontrou end
            }
            while (tokenValue->codigo != fim);
            
            analex(file, true); // Sneaky
            
            return;
        }
        
        sairErroTerminal(file, ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Bloco");
    }
}

void compilaParametrosFormais(FILE* file, int escopo) { // NEXT: analex(..., true);
    token tokenValue;
    
    tokenValue = analex(file, true);
    if (tokenValue->codigo != abreparenteses) {
        return;
    }
    
    while (true) {
        tokenValue = analex(file, true);
        
        if (tokenValue->codigo == variavel || tokenValue->codigo == identificador) {
            if (tokenValue->codigo == variavel) {
                tokenValue = analex(file, true);
            }
            
            do {
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != virgula && tokenValue->codigo != doispontos) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
                
                if (tokenValue->codigo == virgula) {
                    tokenValue = analex(file, true);
                }
            }
            while(tokenValue->codigo != doispontos);
            
            tokenValue = analex(file, true);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue->codigo != pontoevirgula && tokenValue->codigo != fechaparenteses) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue->codigo == funcao) {
            do {
                tokenValue = analex(file, true);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != virgula && tokenValue->codigo != doispontos) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue->codigo != doispontos);
            
            tokenValue = analex(file, true);
            if (tokenValue->codigo != identificador) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            tokenValue = analex(file, true);
            if (tokenValue->codigo != pontoevirgula && tokenValue->codigo != fechaparenteses) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue->codigo == procedimento) {
            do {
                tokenValue = analex(file, true);
                if (tokenValue->codigo != identificador) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue->codigo != virgula &&
                    tokenValue->codigo != pontoevirgula && 
                    tokenValue->codigo != fechaparenteses)
                {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
                }
            }
            while(tokenValue->codigo != pontoevirgula && tokenValue->codigo != fechaparenteses);
            
            if (tokenValue->codigo == pontoevirgula) {
                continue;
            }
        }
        
        if (tokenValue->codigo == fechaparenteses) {
            analex(file, true);
            return;
        }
        
        sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se [VAR], [FUNC], [PROC] ou [ID]");
    }
}

void compilaComando(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = analex(file, true);
    
    if (tokenValue->codigo == numero) {   // Comando Padrão e Adição de Rótulo
        tokenValue = analex(file, true);
        
        if (tokenValue->codigo != doispontos) {
            sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
        }
        
        tokenValue = analex(file, true);
    }
    
    compilaComandoSemRotulo(file, escopo);
}

void compilaComandoSemRotulo(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    tokenValue = analex(file, false);
    
    if (tokenValue->codigo == identificador) { // Atribuição, procedimento e função
        tokenValue = analex(file, true);
        
        if (tokenValue->codigo == abreparenteses) { // Chamada Função
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false);
                
                if (tokenValue->codigo != fechaparenteses && tokenValue->codigo != virgula) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou fechaparenteses");
                }
            }
            while (tokenValue->codigo != fechaparenteses);
            
            analex(file, true); // Sneak
        }
        
        if (tokenValue->codigo == abrecolchetes || tokenValue->codigo == atribuicao) {
            if (tokenValue->codigo == abrecolchetes) {
                do {
                    compilaExpressao(file, escopo);
                    
                    tokenValue = analex(file, false);
                    
                    if (tokenValue->codigo != virgula && tokenValue->codigo != fechacolchetes) {
                        sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                    }
                }
                while(tokenValue->codigo != fechacolchetes);
                
                tokenValue = analex(file, true);
            }
            
            if (tokenValue->codigo != atribuicao) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se atribuicao");
            }
            
            compilaExpressao(file, escopo); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == vapara) {
        tokenValue = analex(file, true);
        
        if (tokenValue->codigo != numero) {
            sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se numero");
        }
        
        analex(file, true); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == inicio) {
        do {
            analex(file, true);
            
            compilaComando(file, escopo + 1);
            
            tokenValue = analex(file, false);
            
            if (tokenValue->codigo != pontoevirgula && tokenValue->codigo != fim) {
                sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se um fim ou pontoevirgula");
            }
        }
        while (tokenValue->codigo != fim);
        
        analex(file, true); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == se) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false);
        if (tokenValue->codigo != entao) {
            sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se entao");
        }
        
        tokenValue = analex(file, true);
        compilaComandoSemRotulo(file, escopo);
        
        tokenValue = analex(file, false); // Gets Skenay
        if (tokenValue->codigo != senao) {
            return;
        }
        
        tokenValue = analex(file, true);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == enquanto) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false);
        if (tokenValue->codigo != faca) {
            sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se faca");
        }
        
        analex(file, true);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se identificador, vapara, se, enquanto ou inicio");
}

void compilaExpressao(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    compilaExpressaoSimples(file, escopo); 
    
    tokenValue = analex(file, false); // Gets sneaky
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
    
    tokenValue = analex(file, true);
    
    if (tokenValue->codigo == mais || tokenValue->codigo == menos) {
        tokenValue = analex(file, true);
    }
    
    compilaTermo(file, escopo);
    
    while (true) {
        tokenValue = analex(file, false);
        
        if (
            tokenValue->codigo == mais ||
            tokenValue->codigo == menos ||
            tokenValue->codigo == ou
        ) {
            analex(file, true);
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
        tokenValue = analex(file, false); // Gets Sneaky
        
        if (
            tokenValue->codigo == vezes ||
            tokenValue->codigo == dividir ||
            tokenValue->codigo == e
        ) {
            analex(file, true);
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
    
    if (tokenValue->codigo == identificador) {
        tokenValue = analex(file, true); // Sneaky
        
        if (tokenValue->codigo == abreparenteses) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false); // Gets Sneaky
                if (tokenValue->codigo != virgula && tokenValue->codigo != fechaparenteses) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != fechaparenteses);
            
            analex(file, true); // Sneaky
        }
        
        if (tokenValue->codigo == abrecolchetes) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false); // Gets Senaky
                if (tokenValue->codigo != virgula && tokenValue->codigo != fechacolchetes) {
                    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != fechacolchetes);
            
            analex(file, true); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == numero) {
        analex(file, true); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == abreparenteses) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false); // Gets Sneaky
        if (tokenValue->codigo != fechaparenteses) {
            sairErroTerminal(file, ERROR_INVALID_TOKEN, "Esperava-se fechaparenteses");
        }
        
        analex(file, true); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == nao) {
        analex(file, true);
        compilaFator(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(file, ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Fator");
}