#include <stdio.h>

#include "error.h"
#include "lista.h"
#include "anasintatico.h"
#include "analexico.h"

string caminhoArquivoAtual;
FILE* arquivoAtualParaCompilar;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "Informe exatamente um arquivo-fonte para compilar");
    }

    caminhoArquivoAtual = argv[1];
    arquivoAtualParaCompilar = fopen(caminhoArquivoAtual, "r");

    if (arquivoAtualParaCompilar == NULL) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "O arquivo não pôde ser encontrado");
    }

    analizeSynSem(arquivoAtualParaCompilar);
    fclose(arquivoAtualParaCompilar);

    sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa Interpretado Com Sucesso");
}
