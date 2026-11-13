#include <stdio.h>

#include "error.h"
#include "lista.h"
#include "anasintatico.h"
#include "analexico.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "Informe exatamente um arquivo-fonte para compilar");
    }

    FILE* atualArquivoParaCompilar = fopen(argv[1], "r");

    if (atualArquivoParaCompilar == NULL) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "O arquivo não pôde ser encontrado");
    }

    anasin(atualArquivoParaCompilar);
    fclose(atualArquivoParaCompilar);

    sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa Interpretado Com Sucesso");
}
