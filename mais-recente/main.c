#include <stdio.h>

#include "analisadortools.h"
#include "error.h"

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
    
    token tokenValor;
    do {
        tokenValor = analex(file);
        
        printf("%s\n", tokenValor.nomeCompilador);
    }
    while (tokenValor.codigoToken != eof);
    
    // int escopoInicial = 0;
    // anasin(file, 0);
}
