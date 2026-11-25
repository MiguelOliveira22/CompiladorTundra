#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "basics.h"
#include "error.h"
#include "lista.h"
#include "tokenlexico.h"
#include "memorymng.h"

static Lista* symbolTableContents = NULL;
static Lista* ponteirosToClose = NULL;

static void cadastrarPonteirosToClose(Elemento ponteiro, void (*d) (Elemento a)) {
    if (ponteirosToClose == NULL) {
        ponteirosToClose = (Lista*) malloc(sizeof(Lista));

        if (ponteirosToClose == NULL) {
            sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
        }

        construirLista(ponteirosToClose, NULL, NULL, NULL);
    }

    PonteiroDestrutivel* novoDado = (PonteiroDestrutivel*) malloc(sizeof(PonteiroDestrutivel));

    if (novoDado == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }

    novoDado->dados = ponteiro;
    novoDado->d = d;

    if (!inserirElementoNoFinalLista(ponteirosToClose, novoDado)) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }
}

static void freeToken(void* tk) {
    Token* token = (Token*) tk;

    free(token->identificador);
    free(token);
}

Token* cadastrarTokenToClose(TokenTipo codigo, string conteudo, bool especial) {
    Token* novoToken = (Token*) malloc(sizeof(Token));

    if (novoToken == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
    }

    cadastrarPonteirosToClose(novoToken, freeToken);

    novoToken->codigo = codigo;
    novoToken->tokenEspecial = especial;

    if (conteudo != NULL) {
        novoToken->identificador = strduplic(conteudo);

        if (novoToken->identificador == NULL) {
            sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "");
        }
    }
    else {
        novoToken->identificador = "\0";
    }

    // printf("%s + Cadastrou %p\n", NOME_COMPONENTE_MEMORYMNG, novoToken);

    return novoToken;
}

void closePonteiros() {
    if (ponteirosToClose == NULL) {
        return;
    }

    NoLista* atual = ponteirosToClose->inicio;
    while (atual != NULL) {
        PonteiroDestrutivel* item = (PonteiroDestrutivel*) atual->info;

        if (item->d != NULL) {
            item->d(item->dados);
        }

        free(item);
        free(atual);
        atual = atual->prox;
    }

    ponteirosToClose->inicio = NULL;
    ponteirosToClose->fim = NULL;
    ponteirosToClose->count = 0;

    // printf("%s - Liberou Ponteiros!\n");
}

static int compararSymbol(Elemento a, Elemento b) {
    Symbol* symbolA = (Symbol*) a;
    Symbol* symbolB = (Symbol*) b;

    int validoEscopo = symbolA->escopo - symbolB->escopo;

    if (validoEscopo == 0) {
        return strcmp(symbolA->identificador, symbolB->identificador);
    }

    return validoEscopo;
}

static void destruirSymbol(Elemento a) {
    Symbol* symbol = (Symbol*) a;

    NoLista* parametro = symbol->parametros->inicio;
    while (parametro != NULL) {
        removerElementoLista(symbol->parametros, parametro);
        parametro = parametro->prox;
    }

    free(symbol);
}

void adicionarSymbolTable(string symbolIdentificador, Token* symbolCategoria, SymbolTipo symbolTipo, Token* referenciaNatureza, int symbolEscopo) {
    if (symbolTableContents == NULL) {
        symbolTableContents = (Lista*) malloc(sizeof(Lista));
        
        if (symbolTableContents == NULL) {
            sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "Não Foi Possível Alocar a Tabela de Símbolos");
        }

        construirLista(symbolTableContents, compararSymbol, mostrarSymbol, destruirSymbol);
    }

    SymbolGenerico* novoSymbol = (SymbolGenerico*) malloc(sizeof(SymbolGenerico));
    if (novoSymbol == NULL) {
        sairErroTerminal(ERROR_INSUFFICIENT_MEMORY_MALLOC, "Não Foi Possível Alocar Um Novo Símbolo");
    }

    novoSymbol->symbolIdentificador = symbolIdentificador;
    novoSymbol->symbolCategoria = symbolCategoria;
    novoSymbol->symbolTipo = symbolTipo;
    novoSymbol->referenciaNatureza = referenciaNatureza;
    novoSymbol->symbolEscopo = symbolEscopo;
    novoSymbol->symbolIdentificadorAlocado = false;
    novoSymbol->symbolDeslocamento = 0;
    novoSymbol->symbolParametroPorReferencia = false;
    novoSymbol->symbolBuiltinIO = false;
    novoSymbol->symbolIndiceParametro = -1;
    novoSymbol->symbolQuantidadeParametros = 0;
    novoSymbol->symbolDeslocamentoResultado = 0;

    novoSymbol->symbolParameters = NULL;

    // Se já existir um símbolo com o mesmo identificador, a lista recusa a inserção
    // (inserirElementoEmOrdemLista retorna false) e simplesmente ignoramos o novo símbolo.
    if (!inserirElementoEmOrdemLista(symbolTableContents, novoSymbol)) {
        free(novoSymbol);

        char mensagemErro[96];
        snprintf(mensagemErro, sizeof(mensagemErro), "Identificador '%s' Já Está Declarado", symbolIdentificador);
        sairErroTerminal(ERROR_IDENTIFIER_ALREADY_DECLARED, mensagemErro);
    }

    // Print só de visualização, pra acompanhar o que está sendo adicionado na tabela.
    printf(
        "%s + '%s' (categoria %s, tipo %s, escopo %d)\n",
        NOME_COMPONENTE_SYMTABLE,
        novoSymbol->symbolIdentificador,
        novoSymbol->symbolCategoria
        novoSymbol->referenciaNatureza != NULL ? novoSymbol->referenciaNatureza->identificador : "desconhecido",
        novoSymbol->symbolEscopo
    );
}


SymbolGenerico* buscarSymbolTable(string symbolIdentificador, int escopo) {
    if (symbolTableContents == NULL) {
        return NULL;
    }

    SymbolGenerico chaveBusca = {0};
    chaveBusca.symbolIdentificador = symbolIdentificador;
    chaveBusca.symbolEscopo = escopo;

    for (int nivel = escopo; nivel >= 0; nivel--) {
        chaveBusca.symbolEscopo = nivel;
        SymbolGenerico* encontrado = (SymbolGenerico*) buscarElementoLista(symbolTableContents, &chaveBusca);
        if (encontrado != NULL) return encontrado;
    }
    return NULL;
}

Token* buscarTokenTipoBase(string nomeTipo) {
    int totalTipos = sizeof(symbolTiposDefinition) / sizeof(Token);

    for (int i = 0; i < totalTipos; i++) {
        if (strcmp(symbolTiposDefinition[i].identificador, nomeTipo) == 0) {
            return &symbolTiposDefinition[i];
        }
    }

    return NULL;
}

// Remove da tabela todos os símbolos que pertencem ao escopo informado.
// Isso deve ser chamado assim que se termina de compilar um bloco (procedure/function),
// pra que variáveis/parâmetros daquele escopo deixem de existir/poder ser usados depois
// que a gente volta pro escopo de fora.
void removerEscopoSymbolTable(int symbolEscopo) {
    if (symbolTableContents == NULL) {
        return;
    }

    NoLista* atual = symbolTableContents->inicio;
    NoLista* anterior = NULL;

    while (atual != NULL) {
        SymbolGenerico* symbolAtual = (SymbolGenerico*) atual->info;

        if (symbolAtual->symbolEscopo == symbolEscopo) {
            NoLista* remover = atual;
            atual = atual->prox;

            if (anterior == NULL) {
                symbolTableContents->inicio = atual;
            }
            else {
                anterior->prox = atual;
            }

            destruirSymbol(symbolAtual);
            free(remover);

            symbolTableContents->count--;
        }
        else {
            anterior = atual;
            atual = atual->prox;
        }
    }
}
