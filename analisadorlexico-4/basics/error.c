#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiladortundra.h"
#include "error.h"
#include "memorymng.h"

void sairErroTerminal(ErrorTipos erro, string message)
{
    closePonteiros();

    if (erro == ERROR_OK_OPERATION_SUCCESS) {
        printf("OK Sucesso - %s\n", message);
    }
    else if (erro >= ERROR_EXCEEDED_IDENTIFIER_SIZE && erro <= ERROR_INVALID_CALL) {
        readLineToEndError(arquivoCompilando);
        printf("%s:%d:%d: Erro %d - %s\n %4d | %s", caminhoArquivoCompilando, currentPosition.linha + 1, currentPosition.coluna + 1, erro, message, currentPosition.linha + 1, currentLine);
    }
    else if (strlen(message) > 0) {
        printf("Erro %d - %s\n", erro, message);
    }
    else {
        printf("Erro %d - Encerrando Execução\n", erro);
    }

    if (arquivoCompilando != NULL) {
        fclose(arquivoCompilando);
    }

    if (arquivoInstrucoes != NULL) {
        fclose(arquivoInstrucoes);
    }

    exit(erro);
}

void printAvisoTerminal(AvisoTipos aviso, string message) {
    if (strlen(message) > 0) {
        printf("Aviso %d - %s\n", aviso, message);
    }
    else {
        printf("Aviso %d - Observação\n", aviso);
    }
}
