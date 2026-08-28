#include <stdio.h>

#include "basics/error.h"
#include "analisador/analexico.h"

int main(int argc, char** argv) {
    if (argc <= 1) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "Nenhum Arquivo Incluido Para Compilar", NULL);
    }

    for (int i = 1; i < argc; i ++) {
        FILE* arquivoParaCompilar = fopen(argv[i], "r");

        if (arquivoParaCompilar == NULL) {
            sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "Um Arquivo Não Pôde Ser Alocado", arquivoParaCompilar);
        }

        // analexico();
        sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa Interpretado Com Sucesso", arquivoParaCompilar);
    }
}