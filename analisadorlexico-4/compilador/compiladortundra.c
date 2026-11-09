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

    string parametro = (string) malloc(100 * sizeof(char));
    if (parametro == NULL) return;

    parametro[0] = '\0'; 

    if (n > 0) {
        strcat(parametro, parametros[0]);
        
        for (int i = 1; i < n; i++) {
            strcat(parametro, ",");
            strcat(parametro, parametros[i]);
        }
    }
    fprintf(file, "%s %s\n", codigoInstrucao[i], parametro);

    free(parametro)
};