#include <stdio.h>
#include <stdlib.h>
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
static void compilaParametrosFormais(FILE* file, int escopo, SymbolGenerico* assinatura);
static void compilaComando(FILE* file, int escopo);
static void compilaComandoSemRotulo(FILE* file, int escopo);
static SymbolTipo compilaExpressao(FILE* file, int escopo);
static SymbolTipo compilaExpressaoAtual(FILE* file, int escopo);
static SymbolTipo compilaExpressaoSimples(FILE* file, int escopo);
static SymbolTipo compilaTermo(FILE* file, int escopo);
static SymbolTipo compilaFator(FILE* file, int escopo);

static void adicionarParametro(SymbolGenerico* assinatura, SymbolTipo tipo) {
    if (assinatura == NULL) { return; }

    SymbolParametro* novo = (SymbolParametro*) malloc(sizeof(SymbolParametro));
    if (novo == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "Não foi possível alocar a assinatura da função");
    }

    novo->tipo = tipo;
    novo->proximo = NULL;

    SymbolParametro** fim = &assinatura->symbolParameters;
    while (*fim != NULL) fim = &(*fim)->proximo;
    *fim = novo;
}

static void exigirTipo(SymbolTipo atual, SymbolTipo esperado, const char* contexto) {
    if (atual != esperado) sairErroTerminal(ERROR_TYPE_MISMATCH, (string) contexto);
}

static bool simboloDaCategoria(SymbolGenerico* symbol, TokenTipo categoria) {
    return symbol != NULL && symbol->symbolCategoria != NULL && symbol->symbolCategoria->codigo == categoria;
}

static void nomeInternoRotulo(char* destino, size_t tamanho, int escopo, const char* numero) {
    snprintf(destino, tamanho, "@label:%d:%s", escopo, numero);
}

static SymbolGenerico* buscarRotuloVisivel(const char* numero, int escopo) {
    char nome[128];
    for (int nivel = escopo; nivel >= 0; nivel--) {
        nomeInternoRotulo(nome, sizeof(nome), nivel, numero);
        SymbolGenerico* rotulo = buscarSymbolTable(nome);
        if (rotulo != NULL) return rotulo;
    }
    return NULL;
}

// Resolve um tipo primitivo ou um alias já declarado. Um identificador qualquer
// (variável, função, programa etc.) não pode ser usado como tipo.
void resolverTipoBase(Token* tokenTipo, SymbolTipo* naturezaResolvida, Token** referenciaResolvida) {
    Token* tipoBase = buscarTokenTipoBase(tokenTipo->identificador);

    if (tipoBase != NULL) {
        *naturezaResolvida = (SymbolTipo) tipoBase->codigo;
        *referenciaResolvida = tipoBase;
        return;
    }

    SymbolGenerico* aliasEncontrado = buscarSymbolTable(tokenTipo->identificador);

    if (simboloDaCategoria(aliasEncontrado, TOKEN_KEYW_TIPO)) {
        *naturezaResolvida = aliasEncontrado->symbolTipo;
        *referenciaResolvida = aliasEncontrado->referenciaNatureza;
        return;
    }

    char mensagem[192];
    snprintf(mensagem, sizeof(mensagem), "Tipo '%s' não declarado (ou identificador não pertence à categoria type)", tokenTipo->identificador);
    sairErroTerminal(ERROR_UNKNOWN_TYPE, mensagem);
}

// Verifica se um identificador usado (variável, parâmetro, procedimento ou função sendo
// chamado) realmente existe na tabela de símbolos. Se ele já saiu de escopo (ou nunca
// foi declarado), buscarSymbolTable não vai encontrar nada e a gente aborta a compilação.
void verificarSymbolTable(Token* tokenIdentificador) {
    if (buscarSymbolTable(tokenIdentificador->identificador) == NULL) {
        if (strcmp(tokenIdentificador->identificador, "read") == 0) {
            sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, "read só está disponível quando 'input' aparece no cabeçalho do programa");
        }
        if (strcmp(tokenIdentificador->identificador, "write") == 0) {
            sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, "write só está disponível quando 'output' aparece no cabeçalho do programa");
        }
        sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, "Identificador Não Declarado (ou Fora de Escopo)");
    }
}

static Token* definicaoToken(TokenTipo codigo) {
    for (int i = 0; i < sizeof(tokenDefinitions) / sizeof(Token); i ++) {
        if (tokenDefinitions[i].codigo == codigo) return &tokenDefinitions[i];
    }
    return NULL;
}

void anasin(FILE* file) {
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
    adicionarSymbolTable(tokenValue->identificador, definicaoToken(TOKEN_KEYW_PROGRAMA), TYPE_BASE_NULL, NULL, (i8) escopo);
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um abre parenteses!");
    }
    
    bool declarouInput = false;
    bool declarouOutput = false;
    while (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
        tokenValue = readNextToken(file);
        if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador!");
        }
        adicionarSymbolTable(tokenValue->identificador, NULL, TYPE_BASE_NULL, NULL, (i8) escopo);
        if (strcmp(tokenValue->identificador, "input") == 0) declarouInput = true;
        if (strcmp(tokenValue->identificador, "output") == 0) declarouOutput = true;
        
        tokenValue = readNextToken(file);
        if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um virgula ou um fecha parenteses!");
        }
    }

    // read precisa da declaração de arquivo "input" no cabeçalho; write precisa de "output".
    Token* categoriaProcedimento = definicaoToken(TOKEN_KEYW_PROCEDIMENTO);
    if (declarouInput) adicionarSymbolTable("read", categoriaProcedimento, TYPE_BASE_NULL, NULL, (i8) escopo);
    if (declarouOutput) adicionarSymbolTable("write", categoriaProcedimento, TYPE_BASE_NULL, NULL, (i8) escopo);
    
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
            Token* categoriaRotulo = tokenValue;
            do {
                tokenValue = readNextToken(file);
                if (tokenValue->codigo != TOKEN_OPER_NUMERO) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um número na declaração de rótulos");
                }

                char nomeRotulo[128];
                nomeInternoRotulo(nomeRotulo, sizeof(nomeRotulo), escopo, tokenValue->identificador);
                size_t tamanhoNome = strlen(nomeRotulo) + 1;
                char* nomePersistente = (char*) malloc(tamanhoNome);
                if (nomePersistente == NULL) {
                    sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "Não foi possível armazenar o identificador do rótulo");
                }
                memcpy(nomePersistente, nomeRotulo, tamanhoNome);
                adicionarSymbolTable(nomePersistente, categoriaRotulo, TYPE_BASE_NULL, NULL, (i8) escopo);
                buscarSymbolTable(nomePersistente)->symbolIdentificadorAlocado = true;
                
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
            SymbolGenerico* simboloProcedimento = buscarSymbolTable(tokenValue->identificador);
            
            compilaParametrosFormais(file, escopo + 1, simboloProcedimento); // Parâmetros pertencem ao escopo de dentro do bloco
            
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

            adicionarSymbolTable(nomeFuncao->identificador, categoriaSymbol, TYPE_BASE_NULL, NULL, (i8) escopo);
            SymbolGenerico* simboloFuncao = buscarSymbolTable(nomeFuncao->identificador);
            
            compilaParametrosFormais(file, escopo + 1, simboloFuncao); // Parâmetros pertencem ao escopo de dentro do bloco
            
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
            simboloFuncao->symbolTipo = naturezaResolvida;
            simboloFuncao->referenciaNatureza = referenciaResolvida;
            
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

void compilaParametrosFormais(FILE* file, int escopo, SymbolGenerico* assinatura) { // NEXT: analex(..., true);
    Token* tokenValue;
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        return;
    }
    
    while (true) {
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL || tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) {
            // Pela regra desta linguagem, parâmetros tipados sem "var" também são por referência.
            Token* categoriaSymbol = &tokenDefinitions[TOKEN_KEYW_VARIAVEL];

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
                adicionarParametro(assinatura, naturezaResolvida);
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
                adicionarParametro(assinatura, TYPE_BASE_NULL);
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
                adicionarParametro(assinatura, TYPE_BASE_NULL);
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

static bool ehBuiltinIO(SymbolGenerico* symbol) {
    return symbol != NULL &&
        (strcmp(symbol->symbolIdentificador, "read") == 0 || strcmp(symbol->symbolIdentificador, "write") == 0);
}

static void compilarArgumentos(FILE* file, int escopo, SymbolGenerico* funcao) {
    Token* tokenValue = readNextToken(file); /* primeiro argumento ou ')' */
    SymbolParametro* parametro = funcao->symbolParameters;
    bool builtinIO = ehBuiltinIO(funcao);
    bool chamavel = simboloDaCategoria(funcao, TOKEN_KEYW_FUNCAO) ||
        simboloDaCategoria(funcao, TOKEN_KEYW_PROCEDIMENTO);

    if (!chamavel && !builtinIO) {
        sairErroTerminal(ERROR_INVALID_CALL, "Este identificador não declara uma função ou procedimento chamável");
    }

    if (tokenValue->codigo == TOKEN_SYMB_FECHAPARENTESES) {
        sairErroTerminal(ERROR_INVALID_CALL, "A lista de argumentos não pode estar vazia; omita os parênteses quando a sub-rotina não tiver parâmetros");
    }

    while (true) {
        SymbolTipo tipoArgumento = compilaExpressaoAtual(file, escopo);
        tokenValue = getCurrentToken();

        if (!builtinIO) {
            if (parametro == NULL) {
                sairErroTerminal(ERROR_ARGUMENT_COUNT, "A chamada recebeu mais argumentos do que a sub-rotina declara");
            }
            if (tipoArgumento != parametro->tipo) {
                sairErroTerminal(ERROR_TYPE_MISMATCH, "O tipo deste argumento não corresponde ao parâmetro declarado");
            }
            parametro = parametro->proximo;
        }

        if (tokenValue->codigo == TOKEN_SYMB_FECHAPARENTESES) break;
        if (tokenValue->codigo != TOKEN_SYMB_VIRGULA) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se vírgula ou fecha parênteses após o argumento da função");
        }
        readNextToken(file);
    }

    if (!builtinIO && parametro != NULL) {
        sairErroTerminal(ERROR_ARGUMENT_COUNT, "A chamada recebeu menos argumentos do que a sub-rotina declara");
    }
    readNextToken(file);
}

void compilaComando(FILE* file, int escopo) {
    Token* tokenValue;
    
    tokenValue = readNextToken(file);
    
    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {   // Comando Padrão e Adição de Rótulo
        char numeroRotulo[128];
        snprintf(numeroRotulo, sizeof(numeroRotulo), "%s", tokenValue->identificador);
        if (buscarRotuloVisivel(numeroRotulo, escopo) == NULL) {
            char mensagem[192];
            snprintf(mensagem, sizeof(mensagem), "Rótulo '%s' usado no comando não foi declarado neste bloco", numeroRotulo);
            sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, mensagem);
        }
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
        SymbolGenerico* simboloAtual = buscarSymbolTable(tokenValue->identificador);
        bool chamadaComParenteses = false;
        
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) { // Chamada Função
            chamadaComParenteses = true;
            compilarArgumentos(file, escopo, simboloAtual);
            tokenValue = getCurrentToken();
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES || tokenValue->codigo == TOKEN_SYMB_ATRIBUICAO) {
            if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES) {
                do {
                    exigirTipo(compilaExpressao(file, escopo), TYPE_BASE_INTEGER, "Índices de acesso devem ser numéricos");
                    
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
            
            SymbolTipo tipoDestino = simboloAtual->symbolTipo;
            exigirTipo(compilaExpressao(file, escopo), tipoDestino, "O tipo do valor atribuído não corresponde ao tipo da variável");
        }

        if (!chamadaComParenteses && simboloDaCategoria(simboloAtual, TOKEN_KEYW_PROCEDIMENTO)) {
            if (simboloAtual->symbolParameters != NULL) {
                sairErroTerminal(ERROR_ARGUMENT_COUNT, "A chamada sem parênteses não fornece os parâmetros declarados pelo procedimento");
            }
        }

        if (simboloDaCategoria(simboloAtual, TOKEN_KEYW_FUNCAO) &&
            simboloAtual->symbolParameters != NULL &&
            tokenValue->codigo != TOKEN_SYMB_ATRIBUICAO) {
            sairErroTerminal(ERROR_ARGUMENT_COUNT, "A função exige argumentos conforme sua declaração");
        }
        
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_VAPARA) {
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo != TOKEN_OPER_NUMERO) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se numero");
        }
        
        char numeroRotulo[128];
        snprintf(numeroRotulo, sizeof(numeroRotulo), "%s", tokenValue->identificador);
        if (buscarRotuloVisivel(numeroRotulo, escopo) == NULL) {
            char mensagem[192];
            snprintf(mensagem, sizeof(mensagem), "goto referencia o rótulo '%s', que não foi declarado neste bloco", numeroRotulo);
            sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, mensagem);
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
        exigirTipo(compilaExpressao(file, escopo), TYPE_BASE_BOOLEAN, "A condição do if precisa ser booleana");
        
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
        exigirTipo(compilaExpressao(file, escopo), TYPE_BASE_BOOLEAN, "A condição do while precisa ser booleana");
        
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

static bool operadorRelacional(TokenTipo codigo) {
    return codigo == TOKEN_SYMB_IGUAL || codigo == TOKEN_SYMB_DIFERENTE ||
        codigo == TOKEN_SYMB_MAIORIGUAL || codigo == TOKEN_SYMB_MENORIGUAL ||
        codigo == TOKEN_SYMB_MAIOR || codigo == TOKEN_SYMB_MENOR;
}

static SymbolTipo compilaExpressaoAtual(FILE* file, int escopo) {
    SymbolTipo esquerda = compilaExpressaoSimples(file, escopo);
    TokenTipo operador = getCurrentToken()->codigo;
    if (!operadorRelacional(operador)) return esquerda;

    if (operador == TOKEN_SYMB_MAIOR || operador == TOKEN_SYMB_MAIORIGUAL ||
        operador == TOKEN_SYMB_MENOR || operador == TOKEN_SYMB_MENORIGUAL) {
        exigirTipo(esquerda, TYPE_BASE_INTEGER, "Comparações de ordem aceitam apenas valores numéricos");
    }

    readNextToken(file);
    SymbolTipo direita = compilaExpressaoSimples(file, escopo);
    if (esquerda != direita) {
        sairErroTerminal(ERROR_TYPE_MISMATCH, "Os tipos dos operandos da comparação são incompatíveis");
    }
    if (operador == TOKEN_SYMB_MAIOR || operador == TOKEN_SYMB_MAIORIGUAL ||
        operador == TOKEN_SYMB_MENOR || operador == TOKEN_SYMB_MENORIGUAL) {
        exigirTipo(direita, TYPE_BASE_INTEGER, "Comparações de ordem aceitam apenas valores numéricos");
    }
    return TYPE_BASE_BOOLEAN;
}

static SymbolTipo compilaExpressao(FILE* file, int escopo) {
    readNextToken(file);
    return compilaExpressaoAtual(file, escopo);
}

static SymbolTipo compilaExpressaoSimples(FILE* file, int escopo) {
    Token* tokenValue = getCurrentToken();
    TokenTipo sinal = tokenValue->codigo;
    if (sinal == TOKEN_SYMB_MAIS || sinal == TOKEN_SYMB_MENOS) tokenValue = readNextToken(file);

    SymbolTipo tipo = compilaTermo(file, escopo);
    if (sinal == TOKEN_SYMB_MAIS || sinal == TOKEN_SYMB_MENOS) {
        exigirTipo(tipo, TYPE_BASE_INTEGER, "O sinal unário aceita apenas valores numéricos");
    }

    while (true) {
        tokenValue = getCurrentToken();
        if (tokenValue->codigo == TOKEN_SYMB_MAIS || tokenValue->codigo == TOKEN_SYMB_MENOS) {
            exigirTipo(tipo, TYPE_BASE_INTEGER, "Operadores aritméticos aceitam apenas valores numéricos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaTermo(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_INTEGER, "Operadores aritméticos aceitam apenas valores numéricos");
            tipo = TYPE_BASE_INTEGER;
        } else if (tokenValue->codigo == TOKEN_SYMB_OU) {
            exigirTipo(tipo, TYPE_BASE_BOOLEAN, "O operador or aceita apenas valores booleanos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaTermo(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_BOOLEAN, "O operador or aceita apenas valores booleanos");
            tipo = TYPE_BASE_BOOLEAN;
        } else {
            return tipo;
        }
    }
}

static SymbolTipo compilaTermo(FILE* file, int escopo) {
    Token* tokenValue;
    SymbolTipo tipo = compilaFator(file, escopo);
    while (true) {
        tokenValue = getCurrentToken();
        if (tokenValue->codigo == TOKEN_SYMB_VEZES || tokenValue->codigo == TOKEN_SYMB_DIVIDIR) {
            exigirTipo(tipo, TYPE_BASE_INTEGER, "Operadores aritméticos aceitam apenas valores numéricos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaFator(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_INTEGER, "Operadores aritméticos aceitam apenas valores numéricos");
            tipo = TYPE_BASE_INTEGER;
        } else if (tokenValue->codigo == TOKEN_SYMB_E) {
            exigirTipo(tipo, TYPE_BASE_BOOLEAN, "O operador and aceita apenas valores booleanos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaFator(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_BOOLEAN, "O operador and aceita apenas valores booleanos");
            tipo = TYPE_BASE_BOOLEAN;
        } else {
            return tipo;
        }
    }
}

static SymbolTipo compilaFator(FILE* file, int escopo) {
    Token* tokenValue;
    tokenValue = getCurrentToken();

    if (tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) {
        verificarSymbolTable(tokenValue);
        SymbolGenerico* simbolo = buscarSymbolTable(tokenValue->identificador);
        tokenValue = readNextToken(file);

        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
            if (!simboloDaCategoria(simbolo, TOKEN_KEYW_FUNCAO)) {
                sairErroTerminal(ERROR_INVALID_CALL, "Somente funções podem ser usadas em expressões; procedimentos não retornam valores");
            }
            compilarArgumentos(file, escopo, simbolo);
            return simbolo->symbolTipo;
        }

        if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES) {
            do {
                exigirTipo(compilaExpressao(file, escopo), TYPE_BASE_INTEGER, "Índices de acesso devem ser numéricos");
                tokenValue = getCurrentToken();
                if (tokenValue->codigo != TOKEN_SYMB_VIRGULA && tokenValue->codigo != TOKEN_SYMB_FECHACOLCHETES) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se uma virgula ou um fechacolchetes");
                }
            }
            while(tokenValue->codigo != TOKEN_SYMB_FECHACOLCHETES);
            
            readNextToken(file);
            return simbolo->symbolTipo;
        }

        if (simboloDaCategoria(simbolo, TOKEN_KEYW_FUNCAO)) {
            if (simbolo->symbolParameters != NULL) {
                sairErroTerminal(ERROR_ARGUMENT_COUNT, "A função exige argumentos conforme sua declaração");
            }
            return simbolo->symbolTipo;
        }
        if (simboloDaCategoria(simbolo, TOKEN_KEYW_PROCEDIMENTO)) {
            sairErroTerminal(ERROR_INVALID_CALL, "Procedimentos não produzem valores que possam participar de expressões");
        }
        return simbolo->symbolTipo;
    }

    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {
        readNextToken(file);
        return TYPE_BASE_INTEGER;
    }

    if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
        SymbolTipo tipo = compilaExpressao(file, escopo);
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_SYMB_FECHAPARENTESES) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fechaparenteses");
        }
        readNextToken(file);
        return tipo;
    }

    if (tokenValue->codigo == TOKEN_SYMB_NAO) {
        readNextToken(file);
        exigirTipo(compilaFator(file, escopo), TYPE_BASE_BOOLEAN, "O operador not aceita apenas valores booleanos");
        return TYPE_BASE_BOOLEAN;
    }

    sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Fator");
    return TYPE_BASE_NULL;
}
