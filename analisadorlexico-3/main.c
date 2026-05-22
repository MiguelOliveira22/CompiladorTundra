#include <stdio.h>

#include "lista.h"
#include "analisadortools.h"
#include "error.h"

int comparaBosta(int* a, int* b) {
    return 1;
}

int main(int argc, char* argv[])
{
    if (argc <= 0) {
        sairErro(NULL, arquivoNaoPassado, "usage: ./parser [ARQUIVO]");
    }
    
    FILE* file;
    file = fopen(argv[1], "r");
    
    if (file == NULL) {
        sairErro(NULL, arquivoNaoCarregado, "Arquivo não pode ser encontrado!");
    }
    
    // int escopoInicial = 0;
    // anasin(file);
    
    ListaLigada l;
    construtorListaLigada(l, comparaBosta);
    
    int r = (*(l.comparar)) ();
    printf("%d", r);
}
