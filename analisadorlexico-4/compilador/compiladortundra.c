#include <stdio.h>
#include <stdlib.h>

#include "compiladortundra.h"
#include "basics.h"

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
    fprintf(file, "%s", codigoInstrucao[i]);

    for (int k = 0; k < n; k++) {
        fprintf(file, "%s%s", (k == 0) ? " " : ",", parametros[k]);
    }

    fprintf(file, "\n");
}