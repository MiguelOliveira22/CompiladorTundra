#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "symboltable.h"
#include "analexico.h"
#include "tokenlexico.h"
#include "error.h"
#include "basics.h"
#include "anasintatico.h"

static void compilaPrograma(FILE* file, int escopo);
static void compilaBloco(FILE* file, int escopo);
static void compilaParametrosFormais(FILE* file, int escopo);
static void compilaComando(FILE* file, int escopo);
static void compilaComandoSemRotulo(FILE* file, int escopo);
static void compilaExpressao(FILE* file, int escopo);
static void compilaExpressaoSimples(FILE* file, int escopo);
static void compilaTermo(FILE* file, int escopo);
static void compilaFator(FILE* file, int escopo);

// Resolve o "tipo base" (natureza) de um identificador de tipo usado numa declaração:
// 1) Primeiro tenta achar entre os tipos-base conhecidos (symbolTiposDefinition: null/integer);
// 2) Se não achar, tenta achar entre os tipos já declarados antes pelo usuário (ex: "type jonas = integer");
// 3) Se ainda assim não achar, cai em TYPE_BASE_NULL (tipo desconhecido).
void resolverTipoBase(Token* tokenTipo, SymbolTipo* naturezaResolvida, Token** referenciaResolvida) {
    Token* tipoBase = buscarTokenTipoBase(tokenTipo->identificador);

    if (tipoBase != NULL) {
        *naturezaResolvida = (SymbolTipo) tipoBase->codigo;
        *referenciaResolvida = tipoBase;
        return;
    }

    SymbolGenerico* aliasEncontrado = buscarSymbolTable(tokenTipo->identificador);

    if (aliasEncontrado != NULL) {
        *naturezaResolvida = aliasEncontrado->symbolTipo;
        *referenciaResolvida = aliasEncontrado->referenciaNatureza;
        return;
    }

    sairErroTerminal(ERROR_UNKNOWN_TYPE, "O Tipo Não Pôde Ser Determinado, Verifique Novamente");
}

// Verifica se um identificador usado (variável, parâmetro, procedimento ou função sendo
// chamado) realmente existe na tabela de símbolos. Se ele já saiu de escopo (ou nunca
// foi declarado), buscarSymbolTable não vai encontrar nada e a gente aborta a compilação.
void verificarSymbolTable(Token* tokenIdentificador) {
    if (buscarSymbolTable(tokenIdentificador->identificador) == NULL) {
        sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, "Identificador Não Declarado (ou Fora de Escopo)");
    }
}

// read/write são procedimentos embutidos da linguagem (não aparecem em nenhuma declaração
// do programa do usuário), então precisam já existir na tabela antes de começar a compilar,
// senão toda chamada a eles seria acusada como "identificador não declarado".
static void registrarBuiltinsSymbolTable() {
    for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i ++) {
        if (tokenDefinitions[i].codigo == TOKEN_KEYW_FUNCAO) {
            adicionarSymbolTable("read", &tokenDefinitions[i], TYPE_BASE_NULL, NULL, 0);
            adicionarSymbolTable("write", &tokenDefinitions[i], TYPE_BASE_NULL, NULL, 0);
            
            return;
        }
    }
}

void anasin(FILE* file) {
    registrarBuiltinsSymbolTable();
    compilaPrograma(file, 0);
}

static void compilaPrograma(FILE* file, int escopo) {
    Token* tokenValue;
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_KEYW_PROGRAMA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um PROGRAM!");
    }
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um IDENTIFICADOR!");
    }
    adicionarSymbolTable(tokenValue->identificador, NULL, TYPE_BASE_NULL, NULL, (i8) escopo);
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um abre parenteses!");
    }
    
    while (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
        tokenValue = readNextToken(file);
        if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador!");
        }
        adicionarSymbolTable(tokenValue->identificador, NULL, TYPE_BASE_NULL, NULL, (i8) escopo); // ex: input, output
        
        tokenValue = readNextToken(file);
        if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um virgula ou um fecha parenteses!");
        }
    }
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula!");
    }
    
    tokenValue = readNextToken(file);
    compilaBloco(file, escopo+1);
    
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_SYMB_PONTO) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto final!");
    }
    
    tokenValue = readNextToken(file);
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
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_NUMERO) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um número");
                }
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se virgula ou ponto e virgula");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA);
            
            readNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_TIPO) {
            Token* categoriaSymbol = tokenValue; // Token "type", marca a categoria do símbolo

            tokenValue = readNextToken(file); // Pega o primeiro identificador
            
            do {
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                Token* nomeTipo = tokenValue; // Nome do novo tipo (ex: "jonas")
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_IGUAL) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um igual");
                }
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                SymbolTipo naturezaResolvida;
                Token* referenciaResolvida;
                resolverTipoBase(tokenValue, &naturezaResolvida, &referenciaResolvida);
                
                adicionarSymbolTable(nomeTipo->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = readNextToken(file); // Sneakar o identificador
            }
            while(tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL) {
            Token* categoriaSymbol = tokenValue;

            readNextToken(file);
            do {
                Token* nomesVariaveis[MAX_VARIAVEIS_POR_DECLARACAO];
                int totalNomes = 0;

                do {
                    tokenValue = getCurrentToken();
                    
                    if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                    }
                    
                    if (totalNomes >= MAX_VARIAVEIS_POR_DECLARACAO) {
                        sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Excedeu o Número de Variáveis Permitido Numa Mesma Declaração");
                    }
                    nomesVariaveis[totalNomes++] = tokenValue; // Guarda o nome pra registrar depois do tipo
                    
                    tokenValue = readNextToken(file);
                    if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                    }
                    
                    if (tokenValue->codigo == TOKEN_SYMB_VIRGULA) {
                        readNextToken(file);
                    }
                }
                while(tokenValue->codigo != TOKEN_SYMB_DOISPONTOS);
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
                }
                
                SymbolTipo naturezaResolvida;
                Token* referenciaResolvida;
                resolverTipoBase(tokenValue, &naturezaResolvida, &referenciaResolvida);
                
                for (int i = 0; i < totalNomes; i++) {
                    adicionarSymbolTable(nomesVariaveis[i]->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
                }
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
                }
                
                tokenValue = readNextToken(file); // Sneakar o identificador
            }
            while(tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_PROCEDIMENTO) {
            Token* categoriaSymbol = tokenValue; // Token "procedure", marca a categoria do símbolo

            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            // Registra o procedimento no escopo ATUAL (de fora), pra poder ser chamado
            // por quem o declarou e por si mesmo (recursão), já que só é removido
            // quando esse escopo de fora terminar.
            adicionarSymbolTable(tokenValue->identificador, categoriaSymbol, TYPE_BASE_NULL, NULL, (i8) escopo);
            
            compilaParametrosFormais(file, escopo + 1); // Parâmetros pertencem ao escopo de dentro do bloco
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
            }
            
            readNextToken(file);
            compilaBloco(file, escopo + 1);
            
            // Saiu do bloco do procedimento: parâmetros e variáveis locais dele não existem mais.
            removerEscopoSymbolTable((i8) (escopo + 1));
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            readNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_FUNCAO) {
            Token* categoriaSymbol = tokenValue; // Token "function", marca a categoria do símbolo

            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            Token* nomeFuncao = tokenValue;
            
            compilaParametrosFormais(file, escopo + 1); // Parâmetros pertencem ao escopo de dentro do bloco
            
            tokenValue = getCurrentToken(); // Pega o doispontos
            if (tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
            }
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            
            SymbolTipo naturezaResolvida;
            Token* referenciaResolvida;
            resolverTipoBase(tokenValue, &naturezaResolvida, &referenciaResolvida);
            
            // Registra a função no escopo ATUAL (de fora), assim como o procedimento,
            // com o tipo de retorno já resolvido.
            adicionarSymbolTable(nomeFuncao->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            readNextToken(file);
            compilaBloco(file, escopo + 1);
            
            // Saiu do bloco da função: parâmetros e variáveis locais dela não existem mais.
            removerEscopoSymbolTable((i8) (escopo + 1));
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            readNextToken(file);
            
            continue;
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_INICIO) { // Verificar aqui
            do {
                compilaComando(file, escopo);
                
                tokenValue = getCurrentToken(); // Verifica se encontrou end
            }
            while (tokenValue->codigo != TOKEN_KEYW_FIM);
            
            readNextToken(file); // Sneaky
            
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Bloco");
    }
}

void compilaParametrosFormais(FILE* file, int escopo) { // NEXT: analex(..., true);
    Token* tokenValue;
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        return;
    }
    
    while (true) {
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL || tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) {
            Token* categoriaSymbol = NULL; // NULL = parâmetro por valor (sem "var" na frente)

            if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL) {
                categoriaSymbol = tokenValue; // Token "var"
                tokenValue = readNextToken(file);
            }
            
            Token* nomesParametros[MAX_VARIAVEIS_POR_DECLARACAO];
            int totalNomes = 0;
            
            do {
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                if (totalNomes >= MAX_VARIAVEIS_POR_DECLARACAO) {
                    sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Excedeu o Número de Parâmetros Permitido Num Mesmo Grupo");
                }
                nomesParametros[totalNomes++] = tokenValue;
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
                
                if (tokenValue->codigo == TOKEN_SYMB_VIRGULA) {
                    tokenValue = readNextToken(file);
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_DOISPONTOS);
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            SymbolTipo naturezaResolvida;
            Token* referenciaResolvida;
            resolverTipoBase(tokenValue, &naturezaResolvida, &referenciaResolvida);
            
            for (int i = 0; i < totalNomes; i++) {
                adicionarSymbolTable(nomesParametros[i]->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
            }
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_FUNCAO) {
            Token* categoriaSymbol = tokenValue; // Token "function"
            Token* nomesParametros[MAX_VARIAVEIS_POR_DECLARACAO];
            int totalNomes = 0;
            
            do {
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                if (totalNomes >= MAX_VARIAVEIS_POR_DECLARACAO) {
                    sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Excedeu o Número de Parâmetros Permitido Num Mesmo Grupo");
                }
                nomesParametros[totalNomes++] = tokenValue;
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um dois pontos");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_DOISPONTOS);
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
            }
            
            SymbolTipo naturezaResolvida;
            Token* referenciaResolvida;
            resolverTipoBase(tokenValue, &naturezaResolvida, &referenciaResolvida);
            
            for (int i = 0; i < totalNomes; i++) {
                adicionarSymbolTable(nomesParametros[i]->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
            }
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
            }
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_KEYW_PROCEDIMENTO) {
            Token* categoriaSymbol = tokenValue; // Token "procedure"
            Token* nomesParametros[MAX_VARIAVEIS_POR_DECLARACAO];
            int totalNomes = 0;
            
            do {
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um [ID]");
                }
                
                if (totalNomes >= MAX_VARIAVEIS_POR_DECLARACAO) {
                    sairErroTerminal(ERROR_EXCEEDED_IDENTIFIER_SIZE, "Excedeu o Número de Parâmetros Permitido Num Mesmo Grupo");
                }
                nomesParametros[totalNomes++] = tokenValue;
                
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA &&
                    tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && 
                    tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES)
                {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula ou um fecha parenteses");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES);
            
            // Parâmetros do tipo "procedure" não têm tipo de retorno: natureza fica desconhecida.
            for (int i = 0; i < totalNomes; i++) {
                adicionarSymbolTable(nomesParametros[i]->identificador, categoriaSymbol, TYPE_BASE_NULL, NULL, (i8) escopo);
            }
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_FECHAPARENTESES) {
            readNextToken(file);
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se [VAR], [FUNC], [PROC] ou [ID]");
    }
}

void compilaComando(FILE* file, int escopo) {
    Token* tokenValue;
    
    tokenValue = readNextToken(file);
    
    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {   // Comando Padrão e Adição de Rótulo
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
        }
        
        tokenValue = readNextToken(file);
    }
    
    compilaComandoSemRotulo(file, escopo);
}

void compilaComandoSemRotulo(FILE* file, int escopo) {
    Token* tokenValue;
    
    tokenValue = getCurrentToken();
    
    if (tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) { // Atribuição, procedimento e função
        verificarSymbolTable(tokenValue); // Garante que esse identificador foi declarado (e ainda está em escopo)
        
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) { // Chamada Função
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = getCurrentToken();
                
                if (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES && tokenValue->codigo != TOKEN_SYMB_VIRGULA) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou fechaparenteses");
                }
            }
            while (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES);
            
            readNextToken(file); // Sneak
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
                
                tokenValue = readNextToken(file);
            }
            
            if (tokenValue->codigo != TOKEN_SYMB_ATRIBUICAO) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se atribuicao");
            }
            
            compilaExpressao(file, escopo); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_VAPARA) {
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo != TOKEN_OPER_NUMERO) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se numero");
        }
        
        readNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_INICIO) {
        do {
            readNextToken(file);
            
            compilaComando(file, escopo + 1);
            
            tokenValue = getCurrentToken();
            
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA && tokenValue->codigo != TOKEN_KEYW_FIM) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um fim ou pontoevirgula");
            }
        }
        while (tokenValue->codigo != TOKEN_KEYW_FIM);
        
        readNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_SE) {
        compilaExpressao(file, escopo);
        
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_KEYW_ENTAO) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se entao");
        }
        
        tokenValue = readNextToken(file);
        compilaComandoSemRotulo(file, escopo);
        
        tokenValue = getCurrentToken(); // Gets Skenay
        if (tokenValue->codigo != TOKEN_KEYW_SENAO) {
            return;
        }
        
        tokenValue = readNextToken(file);
        compilaComandoSemRotulo(file, escopo); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_ENQUANTO) {
        compilaExpressao(file, escopo);
        
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_KEYW_FACA) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se faca");
        }
        
        readNextToken(file);
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
    
    tokenValue = readNextToken(file);
    
    if (tokenValue->codigo == TOKEN_SYMB_MAIS || tokenValue->codigo == TOKEN_SYMB_MENOS) {
        tokenValue = readNextToken(file);
    }
    
    compilaTermo(file, escopo);
    
    while (true) {
        tokenValue = getCurrentToken();
        
        if (
            tokenValue->codigo == TOKEN_SYMB_MAIS ||
            tokenValue->codigo == TOKEN_SYMB_MENOS ||
            tokenValue->codigo == TOKEN_SYMB_OU
        ) {
            readNextToken(file);
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
            readNextToken(file);
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
        verificarSymbolTable(tokenValue); // Garante que esse identificador foi declarado (e ainda está em escopo)
        
        tokenValue = readNextToken(file); // Sneaky
        
        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
            do {
                compilaExpressao(file, escopo);
                
                tokenValue = getCurrentToken(); // Gets Sneaky
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES);
            
            readNextToken(file); // Sneaky
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
            
            readNextToken(file); // Sneaky
        }
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {
        readNextToken(file); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
        compilaExpressao(file, escopo);
        
        tokenValue = getCurrentToken(); // Gets Sneaky
        if (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fechaparenteses");
        }
        
        readNextToken(file); // Sneaky
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_SYMB_NAO) {
        readNextToken(file);
        compilaFator(file, escopo); // Sneaky
        
        return;
    }
    
    sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Fator");
}
