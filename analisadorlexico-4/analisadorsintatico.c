#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "analisadortools.h"
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
    
    analex(file, true);
    compilaBloco(file, escopo);
    
    tokenValue = analex(file, false);
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
        tokenValue = analex(file, false);
        
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
            
            analex(file, true);
            
            continue;
        }
        
        if (tokenValue.codigoToken == tipo) {
            tokenValue = analex(file, true); // Pega o primeiro identificador
            
            do {
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != igual) {
                    sairErro(file, tokenInexperado, "Esperava-se um igual");
                }
                
                // Problema! Precisamos implementar tipos para a tabela de simbolos
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != identificador) {
                    sairErro(file, tokenInexperado, "Esperava-se um identificador");
                }
                
                tokenValue = analex(file, true);
                if (tokenValue.codigoToken != pontoevirgula) {
                    sairErro(file, tokenInexperado, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = analex(file, true); // Sneakar o identificador
            }
            while(tokenValue.codigoToken == identificador);
            
            continue;
        }
        
        if (tokenValue.codigoToken == variavel) {   // Arrumar a virgula
            analex(file, true);
            do {
                do {
                    tokenValue = analex(file, false); // Pega o Sneaky
                    
                    if (tokenValue.codigoToken != identificador) {
                        sairErro(file, tokenInexperado, "Esperava-se um identificador");
                    }
                    
                    tokenValue = analex(file, true);
                    if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != doispontos) {
                        sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um dois pontos");
                    }
                    
                    if (tokenValue.codigoToken == virgula) {
                        analex(file, true);
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
                
                tokenValue = analex(file, true); // Sneakar o identificador
            }
            while(tokenValue.codigoToken == identificador);
            
            continue;
        }
        
        if (tokenValue.codigoToken == procedimento) {
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = analex(file, false); // Pega o Sneaky
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um pontoevirgula");
            }
            
            analex(file, true);
            compilaBloco(file, escopo + 1);
            
            tokenValue = analex(file, false); // Pega o Sneaky
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            }
            
            analex(file, true);
            
            continue;
        }
        
        if (tokenValue.codigoToken == funcao) {
            tokenValue = analex(file, true);
            if (tokenValue.codigoToken != identificador) {
                sairErro(file, tokenInexperado, "Esperava-se um identificador");
            }
            
            compilaParametrosFormais(file, escopo);
            
            tokenValue = analex(file, false); // Pega o doispontos
            if (tokenValue.codigoToken != doispontos) {
                sairErro(file, tokenInexperado, "Esperava-se um doispontos");
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
            
            analex(file, true);
            compilaBloco(file, escopo + 1);
            
            tokenValue = analex(file, false); // Pega o Sneaky
            if (tokenValue.codigoToken != pontoevirgula) {
                sairErro(file, tokenInexperado, "Esperava-se um ponto e virgula");
            }
            
            analex(file, true);
            
            continue;
        }
        
        if (tokenValue.codigoToken == inicio) { // Verificar aqui
            do {
                compilaComando(file, escopo);
                
                tokenValue = analex(file, false); // Verifica se encontrou end
            }
            while (tokenValue.codigoToken != fim);
            
            analex(file, true); // Sneaky
            
            return;
        }
        
        sairErro(file, tokenInexperado, "Sintaxe Inexperada Para Bloco");
    }
}

void compilaParametrosFormais(FILE* file, int escopo) { // NEXT: analex(..., true);
    token tokenValue;
    
    tokenValue = analex(file, true);
    if (tokenValue.codigoToken != abreparenteses) {
        return;
    }
    
    while (true) {
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken == variavel || tokenValue.codigoToken == identificador) {
            if (tokenValue.codigoToken == variavel) {
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
                
                if (tokenValue.codigoToken == virgula) {
                    tokenValue = analex(file, true);
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
            analex(file, true);
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
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false);
                
                if (tokenValue.codigoToken != fechaparenteses && tokenValue.codigoToken != virgula) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou fechaparenteses");
                }
            }
            while (tokenValue.codigoToken != fechaparenteses);
            
            analex(file, true); // Sneak
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
            
            compilaExpressao(file, escopo); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue.codigoToken == vapara) {
        tokenValue = analex(file, true);
        
        if (tokenValue.codigoToken != numero) {
            sairErro(file, tokenInexperado, "Esperava-se numero");
        }
        
        analex(file, true); // Sneaky
        return;
    }
    
    if (tokenValue.codigoToken == inicio) {
        do {
            analex(file, true);
            
            compilaComando(file, escopo + 1);
            
            tokenValue = analex(file, false);
            
            if (tokenValue.codigoToken != pontoevirgula && tokenValue.codigoToken != fim) {
                sairErro(file, tokenInexperado, "Esperava-se um fim ou pontoevirgula");
            }
        }
        while (tokenValue.codigoToken != fim);
        
        analex(file, true); // Sneaky
        return;
    }
    
    if (tokenValue.codigoToken == se) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false);
        if (tokenValue.codigoToken != entao) {
            sairErro(file, tokenInexperado, "Esperava-se entao");
        }
        
        tokenValue = analex(file, true);
        compilaComandoSemRotulo(file, escopo);
        
        tokenValue = analex(file, false); // Gets Skenay
        if (tokenValue.codigoToken != senao) {
            return;
        }
        
        tokenValue = analex(file, true);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    if (tokenValue.codigoToken == enquanto) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false);
        if (tokenValue.codigoToken != faca) {
            sairErro(file, tokenInexperado, "Esperava-se faca");
        }
        
        analex(file, true);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    sairErro(file, tokenInexperado, "Esperava-se identificador, vapara, se, enquanto ou inicio");
}

void compilaExpressao(FILE* file, int escopo) { // NEXT: analex(..., false);
    token tokenValue;
    
    compilaExpressaoSimples(file, escopo); 
    
    tokenValue = analex(file, false); // Gets sneaky
    if (
        tokenValue.codigoToken == igual ||
        tokenValue.codigoToken == diferente ||
        tokenValue.codigoToken == maiorouigual ||
        tokenValue.codigoToken == menorouigual ||
        tokenValue.codigoToken == maior ||
        tokenValue.codigoToken == menor
    ) {
        compilaExpressaoSimples(file, escopo); // Sneaky
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
            tokenValue.codigoToken == vezes ||
            tokenValue.codigoToken == dividir ||
            tokenValue.codigoToken == e
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
    
    if (tokenValue.codigoToken == identificador) {
        tokenValue = analex(file, true); // Sneaky
        
        if (tokenValue.codigoToken == abreparenteses) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false); // Gets Sneaky
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechaparenteses) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue.codigoToken != fechaparenteses);
            
            analex(file, true); // Sneaky
        }
        
        if (tokenValue.codigoToken == abrecolchetes) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = analex(file, false); // Gets Senaky
                if (tokenValue.codigoToken != virgula && tokenValue.codigoToken != fechacolchetes) {
                    sairErro(file, tokenInexperado, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue.codigoToken != fechacolchetes);
            
            analex(file, true); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue.codigoToken == numero) {
        analex(file, true); // Sneaky
        
        return;
    }
    
    if (tokenValue.codigoToken == abreparenteses) {
        compilaExpressao(file, escopo);
        
        tokenValue = analex(file, false); // Gets Sneaky
        if (tokenValue.codigoToken != fechaparenteses) {
            sairErro(file, tokenInexperado, "Esperava-se fechaparenteses");
        }
        
        analex(file, true); // Sneaky
        
        return;
    }
    
    if (tokenValue.codigoToken == nao) {
        analex(file, true);
        compilaFator(file, escopo); // Sneaky
        
        return;
    }
    
    sairErro(file, tokenInexperado, "Sintaxe Inexperada Para Fator");
}