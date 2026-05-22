#include <stdio.h>

#include "lista.h"
#include "analisadortools.h"
#include "error.h"

int comparaBosta(Elemento a, Elemento b) {
    printf("%s", "a");
    int valorA = *(int*)a;
    int valorB = *(int*)b;
    
    if (valorA < valorB) return -1;
    if (valorA > valorB) return 1;
    return 0;
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
    
    ListaLigada* l;
    construtorListaLigada(l, comparaBosta);
    
    Elemento a;
    Elemento b;
    
    int n1 = 1;
    int n2 = 1;

    a = &n1; // 'a' agora aponta para o inteiro 1
    b = &n2; // 'b' agora aponta para o inteiro 2
    
    int r = l->comparar(a, b);
    printf("%d", r);
}
