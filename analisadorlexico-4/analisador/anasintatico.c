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
#include "compiladortundra.h"

static void compilaPrograma(FILE* file, int escopo);
static void compilaBloco(FILE* file, int escopo, SymbolGenerico* dono);
static void compilaParametrosFormais(FILE* file, int escopo, SymbolGenerico* assinatura);
static void compilaComandoAtual(FILE* file, int escopo);
static void compilaComandoSemRotulo(FILE* file, int escopo);
static SymbolTipo compilaExpressao(FILE* file, int escopo);
static SymbolTipo compilaExpressaoAtual(FILE* file, int escopo);
static SymbolTipo compilaExpressaoSimples(FILE* file, int escopo);
static SymbolTipo compilaTermo(FILE* file, int escopo);
static SymbolTipo compilaFator(FILE* file, int escopo);
static int proximoDeslocamentoLocal[128];
static unsigned long contadorRotulosCodigo = 0;
static void emitir(IndiceInstrucao instrucao, const char* a, const char* b, const char* c);
static bool simboloDaCategoria(SymbolGenerico* symbol, TokenTipo categoria);

static void adicionarParametro(SymbolGenerico* assinatura, const char* nome, SymbolTipo tipo, bool porReferencia) {
    if (assinatura == NULL) { return; }

    SymbolParametro* novo = (SymbolParametro*) malloc(sizeof(SymbolParametro));
    if (novo == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "Não foi possível alocar a assinatura da função");
    }

    novo->nome = nome;
    novo->tipo = tipo;
    novo->porReferencia = porReferencia;
    novo->proximo = NULL;

    SymbolParametro** fim = &assinatura->symbolParameters;
    while (*fim != NULL) fim = &(*fim)->proximo;
    *fim = novo;
}

static void parametroEndereco(SymbolGenerico* assinatura, int escopo) {
    int quantidade = 0;
    for (SymbolParametro* p = assinatura->symbolParameters; p != NULL; p = p->proximo) quantidade++;
    assinatura->symbolQuantidadeParametros = quantidade;
    int indice = 0;
    for (SymbolParametro* p = assinatura->symbolParameters; p != NULL; p = p->proximo, indice++) {
        SymbolGenerico* simbolo = buscarSymbolTable((string) p->nome, (i8) escopo);
        if (simbolo != NULL) {
            simbolo->symbolIndiceParametro = indice;
            simbolo->symbolParametroPorReferencia = p->porReferencia;
            simbolo->symbolDeslocamento = -3 - quantidade + indice;
        }
    }
    assinatura->symbolDeslocamentoResultado = -4 - quantidade;
}

static void nomeCodigoSubrotina(char* destino, size_t tamanho, const SymbolGenerico* simbolo) {
    snprintf(destino, tamanho, "PROC_%d_%s", simbolo->symbolEscopo, simbolo->symbolIdentificador);
}

static void emitirChamada(SymbolGenerico* simbolo, int escopoAtual) {
    char nome[256], nivel[16];
    nomeCodigoSubrotina(nome, sizeof(nome), simbolo);
    snprintf(nivel, sizeof(nivel), "%d", escopoAtual - 1);
    emitir(CONTROLE_CHAMADA_PROCEDIMENTO, nome, nivel, NULL);
}

static void novoRotuloCodigo(char* destino, size_t tamanho, const char* prefixo) {
    snprintf(destino, tamanho, "_%s_%lu", prefixo, ++contadorRotulosCodigo);
}

static void emitir(IndiceInstrucao instrucao, const char* a, const char* b, const char* c) {
    string parametros[3];
    int quantidade = 0;
    if (a != NULL) parametros[quantidade++] = (string) a;
    if (b != NULL) parametros[quantidade++] = (string) b;
    if (c != NULL) parametros[quantidade++] = (string) c;
    writeCommand(instrucao, parametros, quantidade);
}

static void emitirAcesso(IndiceInstrucao instrucao, const SymbolGenerico* simbolo) {
    char nivel[16], deslocamento[16];
    snprintf(nivel, sizeof(nivel), "%d", (int) simbolo->symbolEscopo - 1);
    snprintf(deslocamento, sizeof(deslocamento), "%d", simbolo->symbolDeslocamento);
    emitir(instrucao, nivel, deslocamento, NULL);
}

static void emitirCargaSimbolo(const SymbolGenerico* simbolo, int escopoAtual) {
    if (simboloDaCategoria((SymbolGenerico*) simbolo, TOKEN_KEYW_FUNCAO)) {
        char nivel[16], deslocamento[16];
        snprintf(nivel, sizeof(nivel), "%d", escopoAtual - 1);
        snprintf(deslocamento, sizeof(deslocamento), "%d", simbolo->symbolDeslocamentoResultado);
        emitir(CARREGA_VALOR, nivel, deslocamento, NULL);
    } else {
        emitirAcesso(simbolo->symbolParametroPorReferencia ? CARREGA_VALOR_INDIRETO : CARREGA_VALOR, simbolo);
    }
}

static void emitirArmazenamento(const SymbolGenerico* simbolo, int escopoAtual) {
    if (simboloDaCategoria((SymbolGenerico*) simbolo, TOKEN_KEYW_FUNCAO)) {
        char nivel[16], deslocamento[16];
        snprintf(nivel, sizeof(nivel), "%d", escopoAtual - 1);
        snprintf(deslocamento, sizeof(deslocamento), "%d", simbolo->symbolDeslocamentoResultado);
        emitir(ARMAZENA_VALOR, nivel, deslocamento, NULL);
    } else {
        emitirAcesso(simbolo->symbolParametroPorReferencia ? ARMAZENA_VALOR_INDIRETO : ARMAZENA_VALOR, simbolo);
    }
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

static void nomeRotuloCodigo(char* destino, size_t tamanho, int escopo, const char* numero) {
    snprintf(destino, tamanho, "L%d_%s", escopo, numero);
}

static SymbolGenerico* buscarRotuloVisivel(const char* numero, int escopo) {
    char nome[128];
    for (int nivel = escopo; nivel >= 0; nivel--) {
        nomeInternoRotulo(nome, sizeof(nome), nivel, numero);
        SymbolGenerico* rotulo = buscarSymbolTable(nome, (i8) nivel);
        if (rotulo != NULL) return rotulo;
    }
    return NULL;
}

// Resolve um tipo primitivo ou um alias já declarado. Um identificador qualquer
// (variável, função, programa etc.) não pode ser usado como tipo.
void resolverTipoBase(Token* tokenTipo, int escopo, SymbolTipo* naturezaResolvida, Token** referenciaResolvida) {
    Token* tipoBase = buscarTokenTipoBase(tokenTipo->identificador);

    if (tipoBase != NULL) {
        *naturezaResolvida = (SymbolTipo) tipoBase->codigo;
        *referenciaResolvida = tipoBase;
        return;
    }

    SymbolGenerico* aliasEncontrado = buscarSymbolTable(tokenTipo->identificador, (i8) escopo);

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
void verificarSymbolTable(Token* tokenIdentificador, int escopo) {
    if (buscarSymbolTable(tokenIdentificador->identificador, (i8) escopo) == NULL) {
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
    if (declarouInput) {
        adicionarSymbolTable("read", categoriaProcedimento, TYPE_BASE_NULL, NULL, (i8) escopo);
        buscarSymbolTable("read", (i8) escopo)->symbolBuiltinIO = true;
    }
    if (declarouOutput) {
        adicionarSymbolTable("write", categoriaProcedimento, TYPE_BASE_NULL, NULL, (i8) escopo);
        buscarSymbolTable("write", (i8) escopo)->symbolBuiltinIO = true;
    }
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula!");
    }
    
    tokenValue = readNextToken(file);

    writeCommand(CONTROLE_INICIA_PROGRAMA, NULL, 0);
    compilaBloco(file, escopo+1, NULL);
    
    tokenValue = getCurrentToken();
    if (tokenValue->codigo != TOKEN_SYMB_PONTO) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto final!");
    }
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_OPER_EOF) {
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fim de arquivo!");
    }
    
    writeCommand(CONTROLE_PARA, NULL, 0);
    endCompilation();
    return;
}

void compilaBloco(FILE* file, int escopo, SymbolGenerico* dono) {
    Token* tokenValue;
    int totalLocais = 0;
    bool saltoCorpoEmitido = false;
    char rotuloCorpo[256];

    if (escopo >= 128) sairErroTerminal(ERROR_INVALID_TOKEN, "Profundidade de escopo excedida");
    proximoDeslocamentoLocal[escopo] = 0;
    if (dono != NULL) {
        char rotuloProcedimento[256], nivel[16];
        nomeCodigoSubrotina(rotuloProcedimento, sizeof(rotuloProcedimento), dono);
        snprintf(rotuloCorpo, sizeof(rotuloCorpo), "%s_BODY", rotuloProcedimento);
        writeLabel(rotuloProcedimento);
        snprintf(nivel, sizeof(nivel), "%d", escopo - 1);
        emitir(CONTROLE_ENTRADA_PROCEDIMENTO, nivel, NULL, NULL);
    } else {
        snprintf(rotuloCorpo, sizeof(rotuloCorpo), "MAIN");
    }

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
                buscarSymbolTable(nomePersistente, (i8) escopo)->symbolIdentificadorAlocado = true;
                
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
                resolverTipoBase(tokenValue, escopo, &naturezaResolvida, &referenciaResolvida);
                
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

                char bufferNumero[3];
                snprintf(bufferNumero, sizeof(bufferNumero), "%d", totalNomes);
                string params[] = { (string) &bufferNumero };
                writeCommand(CONTROLE_ALOCA_MEM, params, 1);
                
                SymbolTipo naturezaResolvida;
                Token* referenciaResolvida;
                resolverTipoBase(tokenValue, escopo, &naturezaResolvida, &referenciaResolvida);
                
                for (int i = 0; i < totalNomes; i++) {
                    adicionarSymbolTable(nomesVariaveis[i]->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
                    SymbolGenerico* variavel = buscarSymbolTable(nomesVariaveis[i]->identificador, (i8) escopo);
                    variavel->symbolDeslocamento = proximoDeslocamentoLocal[escopo]++;
                    totalLocais++;
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
            if (!saltoCorpoEmitido) {
                emitir(DESVIO_SEMPRE, rotuloCorpo, NULL, NULL);
                saltoCorpoEmitido = true;
            }
            Token* categoriaSymbol = tokenValue; // Token "procedure", marca a categoria do símbolo

            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            // Registra o procedimento no escopo ATUAL (de fora), pra poder ser chamado
            // por quem o declarou e por si mesmo (recursão), já que só é removido
            // quando esse escopo de fora terminar.
            adicionarSymbolTable(tokenValue->identificador, categoriaSymbol, TYPE_BASE_NULL, NULL, (i8) escopo);
            SymbolGenerico* simboloProcedimento = buscarSymbolTable(tokenValue->identificador, (i8) escopo);
            
            compilaParametrosFormais(file, escopo + 1, simboloProcedimento); // Parâmetros pertencem ao escopo de dentro do bloco
            
            tokenValue = getCurrentToken(); // Pega o Sneaky
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um pontoevirgula");
            }
            
            readNextToken(file);
            compilaBloco(file, escopo + 1, simboloProcedimento);
            
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
            if (!saltoCorpoEmitido) {
                emitir(DESVIO_SEMPRE, rotuloCorpo, NULL, NULL);
                saltoCorpoEmitido = true;
            }
            Token* categoriaSymbol = tokenValue; // Token "function", marca a categoria do símbolo

            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um identificador");
            }
            Token* nomeFuncao = tokenValue;

            adicionarSymbolTable(nomeFuncao->identificador, categoriaSymbol, TYPE_BASE_NULL, NULL, (i8) escopo);
            SymbolGenerico* simboloFuncao = buscarSymbolTable(nomeFuncao->identificador, (i8) escopo);
            
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
            resolverTipoBase(tokenValue, escopo, &naturezaResolvida, &referenciaResolvida);
            
            // Registra a função no escopo ATUAL (de fora), assim como o procedimento,
            // com o tipo de retorno já resolvido.
            simboloFuncao->symbolTipo = naturezaResolvida;
            simboloFuncao->referenciaNatureza = referenciaResolvida;
            
            tokenValue = readNextToken(file);
            if (tokenValue->codigo != TOKEN_SYMB_PONTOVIRGULA) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um ponto e virgula");
            }
            
            readNextToken(file);
            compilaBloco(file, escopo + 1, simboloFuncao);
            
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
            if (!saltoCorpoEmitido) {
                emitir(DESVIO_SEMPRE, rotuloCorpo, NULL, NULL);
                saltoCorpoEmitido = true;
            }
            writeLabel(rotuloCorpo);
            tokenValue = readNextToken(file);
            while (tokenValue->codigo != TOKEN_KEYW_FIM) {
                compilaComandoAtual(file, escopo);
                tokenValue = getCurrentToken();
                if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                    tokenValue = readNextToken(file);
                    continue;
                }
                if (tokenValue->codigo != TOKEN_KEYW_FIM) {
                    sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fim ou ponto e vírgula");
                }
            }
            
            readNextToken(file); // Sneaky

            if (totalLocais > 0) {
                char quantidade[16];
                snprintf(quantidade, sizeof(quantidade), "%d", totalLocais);
                emitir(CONTROLE_DELOCA_MEM, quantidade, NULL, NULL);
            }
            if (dono != NULL) {
                char nivel[16], parametros[16];
                snprintf(nivel, sizeof(nivel), "%d", escopo - 1);
                snprintf(parametros, sizeof(parametros), "%d", dono->symbolQuantidadeParametros);
                emitir(CONTROLE_RETORNA_PROCEDIMENTO, nivel, parametros, NULL);
            }
            
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Bloco");
    }
}

void compilaParametrosFormais(FILE* file, int escopo, SymbolGenerico* assinatura) { // NEXT: analex(..., true);
    Token* tokenValue;
    
    tokenValue = readNextToken(file);
    if (tokenValue->codigo != TOKEN_SYMB_ABREPARENTESES) {
        parametroEndereco(assinatura, escopo);
        return;
    }
    
    while (true) {
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo == TOKEN_KEYW_VARIAVEL || tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) {
            bool porReferencia = tokenValue->codigo == TOKEN_KEYW_VARIAVEL;
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
                resolverTipoBase(tokenValue, escopo, &naturezaResolvida, &referenciaResolvida);
            
            for (int i = 0; i < totalNomes; i++) {
                adicionarSymbolTable(nomesParametros[i]->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
                adicionarParametro(assinatura, nomesParametros[i]->identificador, naturezaResolvida, porReferencia);
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
            resolverTipoBase(tokenValue, escopo, &naturezaResolvida, &referenciaResolvida);
            
            for (int i = 0; i < totalNomes; i++) {
                adicionarSymbolTable(nomesParametros[i]->identificador, categoriaSymbol, naturezaResolvida, referenciaResolvida, (i8) escopo);
                adicionarParametro(assinatura, nomesParametros[i]->identificador, TYPE_BASE_NULL, false);
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
                adicionarParametro(assinatura, nomesParametros[i]->identificador, TYPE_BASE_NULL, false);
            }
            
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                continue;
            }
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_FECHAPARENTESES) {
            parametroEndereco(assinatura, escopo);
            readNextToken(file);
            return;
        }
        
        sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se [VAR], [FUNC], [PROC] ou [ID]");
    }
}

static bool ehBuiltinIO(SymbolGenerico* symbol) {
    return symbol != NULL && symbol->symbolBuiltinIO;
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
        SymbolTipo tipoArgumento;
        if (builtinIO && strcmp(funcao->symbolIdentificador, "read") == 0) {
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_CALL, "read aceita apenas variáveis como argumentos");
            }
            verificarSymbolTable(tokenValue, escopo);
            SymbolGenerico* destino = buscarSymbolTable(tokenValue->identificador, (i8) escopo);
            if (!simboloDaCategoria(destino, TOKEN_KEYW_VARIAVEL)) {
                sairErroTerminal(ERROR_INVALID_CALL, "O argumento de read precisa ser uma variável gravável");
            }
            exigirTipo(destino->symbolTipo, TYPE_BASE_INTEGER, "read aceita apenas variáveis do tipo integer");
            tipoArgumento = destino->symbolTipo;
            emitir(CONTROLE_LEITURA, NULL, NULL, NULL);
            emitirArmazenamento(destino, escopo);
            tokenValue = readNextToken(file);
        } else if (!builtinIO && parametro != NULL && parametro->porReferencia) {
            if (tokenValue->codigo != TOKEN_OPER_IDENTIFICADOR) {
                sairErroTerminal(ERROR_INVALID_CALL, "O parâmetro var exige um argumento variável");
            }
            verificarSymbolTable(tokenValue, escopo);
            SymbolGenerico* argumento = buscarSymbolTable(tokenValue->identificador, (i8) escopo);
            if (!simboloDaCategoria(argumento, TOKEN_KEYW_VARIAVEL)) {
                sairErroTerminal(ERROR_INVALID_CALL, "O parâmetro var exige um argumento gravável");
            }
            tipoArgumento = argumento->symbolTipo;
            if (argumento->symbolParametroPorReferencia) emitirAcesso(CARREGA_VALOR, argumento);
            else emitirAcesso(CARREGA_ENDERECO, argumento);
            tokenValue = readNextToken(file);
        } else {
            tipoArgumento = compilaExpressaoAtual(file, escopo);
            if (builtinIO) {
                exigirTipo(tipoArgumento, TYPE_BASE_INTEGER, "write aceita apenas expressões do tipo integer");
                emitir(CONTROLE_IMPRIME, NULL, NULL, NULL);
            }
            tokenValue = getCurrentToken();
        }

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

static void compilaComandoAtual(FILE* file, int escopo) {
    Token* tokenValue = getCurrentToken();
    
    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {   // Comando Padrão e Adição de Rótulo
        char numeroRotulo[128];
        snprintf(numeroRotulo, sizeof(numeroRotulo), "%s", tokenValue->identificador);
        SymbolGenerico* simboloRotulo = buscarRotuloVisivel(numeroRotulo, escopo);
        if (simboloRotulo == NULL) {
            char mensagem[192];
            snprintf(mensagem, sizeof(mensagem), "Rótulo '%s' usado no comando não foi declarado neste bloco", numeroRotulo);
            sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, mensagem);
        }
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo != TOKEN_SYMB_DOISPONTOS) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se um doispontos");
        }

        char rotuloCodigo[160];
        nomeRotuloCodigo(rotuloCodigo, sizeof(rotuloCodigo), simboloRotulo->symbolEscopo, numeroRotulo);
        writeLabel(rotuloCodigo);
        
        tokenValue = readNextToken(file);
    }
    
    compilaComandoSemRotulo(file, escopo);
}

void compilaComandoSemRotulo(FILE* file, int escopo) {
    Token* tokenValue;
    
    tokenValue = getCurrentToken();
    
    if (tokenValue->codigo == TOKEN_OPER_IDENTIFICADOR) { // Atribuição, procedimento e função
        verificarSymbolTable(tokenValue, escopo); // Garante que esse identificador foi declarado (e ainda está em escopo)
        SymbolGenerico* simboloAtual = buscarSymbolTable(tokenValue->identificador, (i8) escopo);
        bool chamadaComParenteses = false;
        bool atribuicao = false;
        
        tokenValue = readNextToken(file);
        
        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) { // Chamada Função
            chamadaComParenteses = true;
            bool chamadaFuncao = simboloDaCategoria(simboloAtual, TOKEN_KEYW_FUNCAO);
            if (chamadaFuncao) emitir(CONTROLE_ALOCA_MEM, "1", NULL, NULL);
            compilarArgumentos(file, escopo, simboloAtual);
            if (!ehBuiltinIO(simboloAtual)) emitirChamada(simboloAtual, escopo);
            tokenValue = getCurrentToken();
            if (chamadaFuncao && tokenValue->codigo != TOKEN_SYMB_ATRIBUICAO) {
                sairErroTerminal(ERROR_INVALID_CALL, "Uma função só pode ser chamada numa expressão");
            }
        }
        
        if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES || tokenValue->codigo == TOKEN_SYMB_ATRIBUICAO) {
            if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES) {
                sairErroTerminal(ERROR_INVALID_CALL, "Variáveis indexadas não são suportadas pela representação atual");
            }
            
            if (tokenValue->codigo != TOKEN_SYMB_ATRIBUICAO) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se atribuicao");
            }
            atribuicao = true;

            bool atribuicaoResultadoFuncao = simboloDaCategoria(simboloAtual, TOKEN_KEYW_FUNCAO) &&
                escopo == (int) simboloAtual->symbolEscopo + 1;
            if (!simboloDaCategoria(simboloAtual, TOKEN_KEYW_VARIAVEL) && !atribuicaoResultadoFuncao) {
                sairErroTerminal(ERROR_INVALID_CALL, "A atribuição exige uma variável ou o resultado da função atual");
            }
            
            SymbolTipo tipoDestino = simboloAtual->symbolTipo;
            exigirTipo(compilaExpressao(file, escopo), tipoDestino, "O tipo do valor atribuído não corresponde ao tipo da variável");
            emitirArmazenamento(simboloAtual, escopo);
        }

        if (!chamadaComParenteses && simboloDaCategoria(simboloAtual, TOKEN_KEYW_PROCEDIMENTO)) {
            if (ehBuiltinIO(simboloAtual)) {
                sairErroTerminal(ERROR_INVALID_CALL, "read e write precisam de parênteses e argumentos");
            }
            if (simboloAtual->symbolParameters != NULL) {
                sairErroTerminal(ERROR_ARGUMENT_COUNT, "A chamada sem parênteses não fornece os parâmetros declarados pelo procedimento");
            }
            emitirChamada(simboloAtual, escopo);
        }

        if (!chamadaComParenteses && simboloDaCategoria(simboloAtual, TOKEN_KEYW_FUNCAO) && tokenValue->codigo != TOKEN_SYMB_ATRIBUICAO) {
            sairErroTerminal(ERROR_INVALID_CALL, "Uma função só pode ser chamada numa expressão ou usada no lado esquerdo do resultado");
        }

        if (!chamadaComParenteses && !atribuicao &&
            !simboloDaCategoria(simboloAtual, TOKEN_KEYW_PROCEDIMENTO) &&
            !simboloDaCategoria(simboloAtual, TOKEN_KEYW_FUNCAO)) {
            sairErroTerminal(ERROR_INVALID_CALL, "Um identificador usado como comando precisa ser chamado ou receber uma atribuição");
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
        SymbolGenerico* simboloRotulo = buscarRotuloVisivel(numeroRotulo, escopo);
        if (simboloRotulo == NULL) {
            char mensagem[192];
            snprintf(mensagem, sizeof(mensagem), "goto referencia o rótulo '%s', que não foi declarado neste bloco", numeroRotulo);
            sairErroTerminal(ERROR_UNDECLARED_IDENTIFIER, mensagem);
        }
        char rotuloCodigo[160], nivelDestino[16], nivelAtual[16];
        nomeRotuloCodigo(rotuloCodigo, sizeof(rotuloCodigo), simboloRotulo->symbolEscopo, numeroRotulo);
        snprintf(nivelDestino, sizeof(nivelDestino), "%d", (int) simboloRotulo->symbolEscopo - 1);
        snprintf(nivelAtual, sizeof(nivelAtual), "%d", escopo - 1);
        emitir(DESVIO_ROTULO, rotuloCodigo, nivelDestino, nivelAtual);
        readNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_INICIO) {
        tokenValue = readNextToken(file);
        while (tokenValue->codigo != TOKEN_KEYW_FIM) {
            compilaComandoAtual(file, escopo);
            tokenValue = getCurrentToken();
            if (tokenValue->codigo == TOKEN_SYMB_PONTOVIRGULA) {
                tokenValue = readNextToken(file);
                continue;
            }
            if (tokenValue->codigo != TOKEN_KEYW_FIM) {
                sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se fim ou ponto e vírgula");
            }
        }
        
        readNextToken(file); // Sneaky
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_SE) {
        exigirTipo(compilaExpressao(file, escopo), TYPE_BASE_BOOLEAN, "A condição do if precisa ser booleana");
        
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_KEYW_ENTAO) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se entao");
        }
        
        char rotuloSenao[64], rotuloFim[64];
        novoRotuloCodigo(rotuloSenao, sizeof(rotuloSenao), "ELSE");
        novoRotuloCodigo(rotuloFim, sizeof(rotuloFim), "ENDIF");
        tokenValue = readNextToken(file);
        emitir(DESVIO_FALSO, rotuloSenao, NULL, NULL);
        compilaComandoAtual(file, escopo);
        
        tokenValue = getCurrentToken(); // Gets Skenay
        if (tokenValue->codigo != TOKEN_KEYW_SENAO) {
            writeLabel(rotuloSenao);
            return;
        }
        
        emitir(DESVIO_SEMPRE, rotuloFim, NULL, NULL);
        writeLabel(rotuloSenao);
        tokenValue = readNextToken(file);
        compilaComandoAtual(file, escopo); // Sneaky
        writeLabel(rotuloFim);
        return;
    }
    
    if (tokenValue->codigo == TOKEN_KEYW_ENQUANTO) {
        char rotuloInicio[64], rotuloFim[64];
        novoRotuloCodigo(rotuloInicio, sizeof(rotuloInicio), "WHILE");
        novoRotuloCodigo(rotuloFim, sizeof(rotuloFim), "ENDWHILE");
        writeLabel(rotuloInicio);
        exigirTipo(compilaExpressao(file, escopo), TYPE_BASE_BOOLEAN, "A condição do while precisa ser booleana");
        
        tokenValue = getCurrentToken();
        if (tokenValue->codigo != TOKEN_KEYW_FACA) {
            sairErroTerminal(ERROR_INVALID_TOKEN, "Esperava-se faca");
        }
        
        readNextToken(file);
        emitir(DESVIO_FALSO, rotuloFim, NULL, NULL);
        compilaComandoAtual(file, escopo); // Sneaky
        emitir(DESVIO_SEMPRE, rotuloInicio, NULL, NULL);
        writeLabel(rotuloFim);
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
    switch (operador) {
        case TOKEN_SYMB_IGUAL: emitir(COMPARA_IGUAL, NULL, NULL, NULL); break;
        case TOKEN_SYMB_DIFERENTE: emitir(COMPARA_DESIGUAL, NULL, NULL, NULL); break;
        case TOKEN_SYMB_MENOR: emitir(COMPARA_MENOR, NULL, NULL, NULL); break;
        case TOKEN_SYMB_MAIOR: emitir(COMPARA_MAIOR, NULL, NULL, NULL); break;
        case TOKEN_SYMB_MENORIGUAL: emitir(COMPARA_MENOR_IGUAL, NULL, NULL, NULL); break;
        case TOKEN_SYMB_MAIORIGUAL: emitir(COMPARA_MAIOR_IGUAL, NULL, NULL, NULL); break;
        default: break;
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
        if (sinal == TOKEN_SYMB_MENOS) emitir(OPERA_INVERTE_SINAL, NULL, NULL, NULL);
    }

    while (true) {
        tokenValue = getCurrentToken();
        if (tokenValue->codigo == TOKEN_SYMB_MAIS || tokenValue->codigo == TOKEN_SYMB_MENOS) {
            exigirTipo(tipo, TYPE_BASE_INTEGER, "Operadores aritméticos aceitam apenas valores numéricos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaTermo(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_INTEGER, "Operadores aritméticos aceitam apenas valores numéricos");
            emitir(tokenValue->codigo == TOKEN_SYMB_MAIS ? OPERA_SOMA : OPERA_SUBTRACAO, NULL, NULL, NULL);
            tipo = TYPE_BASE_INTEGER;
        } else if (tokenValue->codigo == TOKEN_SYMB_OU) {
            exigirTipo(tipo, TYPE_BASE_BOOLEAN, "O operador or aceita apenas valores booleanos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaTermo(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_BOOLEAN, "O operador or aceita apenas valores booleanos");
            emitir(OPERA_DISJUNCAO, NULL, NULL, NULL);
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
            emitir(tokenValue->codigo == TOKEN_SYMB_VEZES ? OPERA_MULTIPLICACAO : OPERA_DIVISAO, NULL, NULL, NULL);
            tipo = TYPE_BASE_INTEGER;
        } else if (tokenValue->codigo == TOKEN_SYMB_E) {
            exigirTipo(tipo, TYPE_BASE_BOOLEAN, "O operador and aceita apenas valores booleanos");
            readNextToken(file);
            SymbolTipo tipoDireito = compilaFator(file, escopo);
            exigirTipo(tipoDireito, TYPE_BASE_BOOLEAN, "O operador and aceita apenas valores booleanos");
            emitir(OPERA_CONJUNCAO, NULL, NULL, NULL);
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
        verificarSymbolTable(tokenValue, escopo);
        SymbolGenerico* simbolo = buscarSymbolTable(tokenValue->identificador, (i8) escopo);
        tokenValue = readNextToken(file);

        if (tokenValue->codigo == TOKEN_SYMB_ABREPARENTESES) {
            if (!simboloDaCategoria(simbolo, TOKEN_KEYW_FUNCAO)) {
                sairErroTerminal(ERROR_INVALID_CALL, "Somente funções podem ser usadas em expressões; procedimentos não retornam valores");
            }
            emitir(CONTROLE_ALOCA_MEM, "1", NULL, NULL);
            compilarArgumentos(file, escopo, simbolo);
            emitirChamada(simbolo, escopo);
            return simbolo->symbolTipo;
        }

        if (tokenValue->codigo == TOKEN_SYMB_ABRECOLCHETES) {
            sairErroTerminal(ERROR_INVALID_CALL, "Variáveis indexadas não são suportadas pela representação atual");
        }

        if (simboloDaCategoria(simbolo, TOKEN_KEYW_FUNCAO)) {
            if (simbolo->symbolParameters != NULL) {
                sairErroTerminal(ERROR_ARGUMENT_COUNT, "A função exige argumentos conforme sua declaração");
            }
            emitir(CONTROLE_ALOCA_MEM, "1", NULL, NULL);
            emitirChamada(simbolo, escopo);
            return simbolo->symbolTipo;
        }
        if (simboloDaCategoria(simbolo, TOKEN_KEYW_PROCEDIMENTO)) {
            sairErroTerminal(ERROR_INVALID_CALL, "Procedimentos não produzem valores que possam participar de expressões");
        }
        emitirCargaSimbolo(simbolo, escopo);
        return simbolo->symbolTipo;
    }

    if (tokenValue->codigo == TOKEN_OPER_NUMERO) {
        emitir(CARREGA_CONSTANTE, tokenValue->identificador, NULL, NULL);
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
        emitir(OPERA_NEGACAO, NULL, NULL, NULL);
        return TYPE_BASE_BOOLEAN;
    }

    sairErroTerminal(ERROR_INVALID_TOKEN, "Sintaxe Inexperada Para Fator");
    return TYPE_BASE_NULL;
}
