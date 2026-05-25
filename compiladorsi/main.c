#include <stdio.h>
#include <stdlib.h>

#include "analisadortools.h"
#include "error.h"

int main(int argc, char* argv[])
{
    // Correção do índice de argumentos: se argc < 2, nenhum ficheiro foi passado
    if (argc < 2) {
        sairErro(NULL, arquivoNaoPassado, "Uso: ./parser [ARQUIVO]\n");
    }
    
    FILE* file;
    file = fopen(argv[1], "r");
    
    if (file == NULL) {
        sairErro(NULL, arquivoNaoCarregado, "Arquivo nao pode ser encontrado!\n");
    }
    
    // Ativação do Analisador Sintático
    anasin(file);
    
    fclose(file);
    return 0;
}