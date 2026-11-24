#include <stdio.h>

#include "error.h"
#include "anasintatico.h"
#include "compiladortundra.h"

string nomeArquivoInstrucoes = "";

int main(int argc, char* argv[]) {
    if (argc != 3) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "Informe exatamente um arquivo-fonte e um arquivo-objeto para compilar");
    }

    FILE* atualArquivoParaCompilar = fopen(argv[1], "r");

    if (atualArquivoParaCompilar == NULL) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "O arquivo não pôde ser encontrado");
    }

    nomeArquivoInstrucoes = argv[2];

    anasin(atualArquivoParaCompilar);
    fclose(atualArquivoParaCompilar);

    sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa Interpretado Com Sucesso");
}
