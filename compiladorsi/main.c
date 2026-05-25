#include <stdio.h>
#include <stdlib.h>

#include "lista.h"
#include "analisadortools.h"
#include "error.h"

int comparaBosta(Elemento a, Elemento b) {
    int valorA = *(int*)a;
    int valorB = *(int*)b;
    
    if (valorA < valorB) return -1;
    if (valorA > valorB) return 1;
    return 0;
}

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