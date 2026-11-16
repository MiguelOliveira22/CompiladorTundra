#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "analexico.h"
#include "error.h"
#include "memorymng.h"

void sairErroTerminal(ErrorTipos erro, string message)
{
    closePonteiros();

    if (erro == ERROR_OK_OPERATION_SUCCESS) {
        printf("OK Sucesso - %s\n", message);
    }
    else if (erro >= ERROR_EXCEEDED_IDENTIFIER_SIZE && erro <= ERROR_INVALID_CALL) {
        readLineToEndError(arquivoAtualParaCompilar);
        printf("%s:%d:%d: Erro %d - %s\n %4d | %s", caminhoArquivoAtual, currentPosition.linha + 1, currentPosition.coluna + 1, erro, message, currentPosition.linha + 1, currentLine);
    }
    else if (strlen(message) > 0) {
        printf("Erro %d - %s\n", erro, message);
    }
    else {
        printf("Erro %d - Encerrando Execução\n", erro);
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

string strduplic(string data) {
    if (data == NULL) {
        return NULL;
    }

    int size = (strlen(data) + 1) * sizeof(char);
    string novo = (string) malloc(size);

    if (novo == NULL) {
        return NULL;
    }

    memcpy(novo, data, size);
    return novo;
}
