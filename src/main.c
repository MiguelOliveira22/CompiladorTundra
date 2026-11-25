#include <stdio.h>

#include "compiladortundra.h"
#include "error.h"
#include "lista.h"
#include "anasintatico.h"
#include "analexico.h"

string caminhoArquivoCompilando;
FILE* arquivoCompilando;

string caminhoArquivoInstrucoes;
FILE* arquivoInstrucoes;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "Informe exatamente um arquivo-fonte e um arquivo-objeto para imprimir!");
    }

    caminhoArquivoCompilando = argv[1];
    arquivoCompilando = fopen(caminhoArquivoCompilando, "r");

    caminhoArquivoInstrucoes = argv[2];
    arquivoInstrucoes = fopen(caminhoArquivoInstrucoes, "w");

    if (arquivoCompilando == NULL) {
        sairErroTerminal(ERROR_FILE_NO_PROGRAM_FILES, "O arquivo-fonte não pôde ser encontrado");
    }

    if (arquivoInstrucoes == NULL) {
        sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "O arquivo-objeto não pôde ser alocado");
    }

    analizeSynSem(arquivoCompilando);
    sairErroTerminal(ERROR_OK_OPERATION_SUCCESS, "Programa Compilado Com Sucesso");
}
