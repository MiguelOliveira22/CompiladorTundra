#include <stdio.h>

#include "error.h"
#include "lista.h"
#include "anasintatico.h"
#include "analexico.h"

static Lista* tabelaSimbolos = NULL;

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "Nenhum Arquivo Incluido Para Compilar");
    }

    // construirLista(tabelaSimbolos, );

    for (int i = 1; i < argc; i ++) {
        FILE* atualArquivoParaCompilar = fopen(argv[i], "r");

        if (atualArquivoParaCompilar == NULL) {
            sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "Um Arquivo Não Pôde Ser Alocado");
        }

        /*while (getCurrentToken() != &tokenEof) {
            readNextToken(atualArquivoParaCompilar);
            printf("[codigo=%d] %s\n", getCurrentToken()->codigo, getCurrentToken()->identificador);
        }*/

        anasin(atualArquivoParaCompilar);
        sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa Interpretado Com Sucesso");
    }
}