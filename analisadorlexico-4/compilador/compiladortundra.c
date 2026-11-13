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

FILE* file;

void writeCommand(IndiceInstrucao i, string parametros[], int n) {
    if (file == NULL) {
        file = fopen("tundrafiles/compfiles/resultado.tpb", "w");
    }

    if (file == NULL) {
        sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "Programa não foi capaz de alocar o arquivo de objeto");
    }

    fprintf(file, "%s", codigoInstrucao[i]);

    for (int k = 0; k < n; k++) {
        fprintf(file, "%s%s", (k == 0) ? " " : ",", parametros[k]);
    }

    fprintf(file, "\n");
}

void writeLabel(const char* label) {
    if (file == NULL) {
        file = fopen("tundrafiles/compfiles/resultado.tpb", "w");
    }
    if (file == NULL) {
        sairErroTerminal(ERROR_FILE_PROGRAM_FILE_NOT_ALLOCATED, "Programa não foi capaz de alocar o arquivo de objeto");
    }
    fprintf(file, "%s:\n", label);
}

void endCompilation() {
    if (file != NULL) {
        fclose(file);
        file = NULL;
    }
}
