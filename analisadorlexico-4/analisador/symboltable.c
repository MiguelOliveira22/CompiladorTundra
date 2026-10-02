#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symboltable.h"
#include "tokenlexico.h"
#include "basics.h"
#include "lista.h"
#include "error.h"

Token symbolTiposDefinition[] = {
    { (TokenTipo) TYPE_BASE_NULL,    "null",    false },
    { (TokenTipo) TYPE_BASE_INTEGER, "integer", false },
    { (TokenTipo) TYPE_BASE_BOOLEAN, "boolean", false },
};

static Lista* symbolTableContents = NULL;

static int compararSymbol(Elemento a, Elemento b) {
    SymbolGenerico* symbolA = (SymbolGenerico*) a;
    SymbolGenerico* symbolB = (SymbolGenerico*) b;

    return strcmp(symbolA->symbolIdentificador, symbolB->symbolIdentificador);
}

static void mostrarSymbol(Elemento a) {
    SymbolGenerico* symbol = (SymbolGenerico*) a;

    printf(
        "[ID] %s;\t[TIPO] %s;\t[CATEG] %s;\n[ESCOPO] %d;\n[PARAMS] ",
        symbol->symbolIdentificador,
        symbol->referenciaNatureza != NULL ? symbol->referenciaNatureza->identificador : "?",
        symbol->symbolCategoria != NULL ? symbol->symbolCategoria->identificador : "?",
        symbol->symbolEscopo
    );
}

static void destruirSymbol(Elemento a) {
    SymbolGenerico* symbol = (SymbolGenerico*) a;
    if (symbol->symbolIdentificadorAlocado) free(symbol->symbolIdentificador);
    SymbolParametro* parametro = symbol->symbolParameters;
    while (parametro != NULL) {
        SymbolParametro* proximo = parametro->proximo;
        free(parametro);
        parametro = proximo;
    }
    free(symbol);
}

void adicionarSymbolTable(string symbolIdentificador, Token* symbolCategoria, SymbolTipo symbolTipo, Token* referenciaNatureza, i8 symbolEscopo) {
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
        "[symtable] + '%s' (categoria=%s, natureza=%s, escopo=%d)\n",
        novoSymbol->symbolIdentificador,
        novoSymbol->symbolCategoria != NULL ? novoSymbol->symbolCategoria->identificador : "?",
        novoSymbol->referenciaNatureza != NULL ? novoSymbol->referenciaNatureza->identificador : "desconhecido",
        novoSymbol->symbolEscopo
    );
}


SymbolGenerico* buscarSymbolTable(string symbolIdentificador) {
    if (symbolTableContents == NULL) {
        return NULL;
    }

    SymbolGenerico chaveBusca;
    chaveBusca.symbolIdentificador = symbolIdentificador;

    return (SymbolGenerico*) buscarElementoLista(symbolTableContents, &chaveBusca);
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
void removerEscopoSymbolTable(i8 symbolEscopo) {
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

            // Print só de visualização, pra acompanhar o que está saindo da tabela.
            printf("[symtable] - '%s' (escopo %d encerrado)\n", symbolAtual->symbolIdentificador, symbolAtual->symbolEscopo);

            if (symbolAtual->symbolIdentificadorAlocado) free(symbolAtual->symbolIdentificador);
            free(symbolAtual);
            free(remover);
            symbolTableContents->count--;
        }
        else {
            anterior = atual;
            atual = atual->prox;
        }
    }
}
