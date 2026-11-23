#include <stdio.h>
#include <stdlib.h>

#include "compiladortundra.h"
#include "basics.h"
#include "error.h"

string codigoInstrucao[] = {
    "CRCT",
    "CRVL",
    "CRVI",
    "CREN",

    "ARMZ",
    "ARMI",

    "SOMA",
    "SUBT",
    "MULT",
    "DIVI",

    "INVR",

    "CONJ",
    "DISJ",
    "NEGA",

    "CMME",
    "CMMA",
    "CMIG",
    "CMDG",
    "CMEG",
    "CMAG",

    "DSVS",
    "DSVF",
    "DSVR",
    
    "NADA",
    "PARA",
    "LEIT",
    "IMPR",
    "AMEM",
    "DMEM",
    "INPP",
    "ENRT",

    "CHPR",
    "ENPR",
    "RTPR"
};

void writeCommand(IndiceInstrucao i, string parametros[], int n) {
    if (arquivoInstrucoes == NULL) {
        sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "Programa não foi capaz de encontrar o arquivo-objeto");
    }

    fprintf(arquivoInstrucoes, "%s", codigoInstrucao[i]);

    for (int k = 0; k < n; k++) {
        fprintf(arquivoInstrucoes, "%s%s", (k == 0) ? " " : ",", parametros[k]);
    }

    fprintf(arquivoInstrucoes, "\n");
}

void writeLabel(string label) {
    if (arquivoInstrucoes == NULL) {
        sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "Programa não foi capaz de encontrar o arquivo-objeto");
    }
    
    fprintf(arquivoInstrucoes, "%s: NADA\n", label);
}
