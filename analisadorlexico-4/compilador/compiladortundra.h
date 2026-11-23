#ifndef compiladortundra
#define compiladortundra

#include "basics.h"

#define CAP_SIZE_LINHA_BUFFER (256 * sizeof(char))

typedef enum {
    INST_CARREGA_CONSTANTE,
    INST_CARREGA_VALOR,
    INST_CARREGA_VALOR_INDIRETO,
    INST_CARREGA_ENDERECO,

    INST_ARMAZENA_VALOR,
    INST_ARMAZENA_VALOR_INDIRETO,

    INST_OPERA_SOMA,
    INST_OPERA_SUBTRACAO,
    INST_OPERA_MULTIPLICACAO,
    INST_OPERA_DIVISAO,

    INST_OPERA_INVERTE_SINAL,

    INST_OPERA_CONJUNCAO,
    INST_OPERA_DISJUNCAO,
    INST_OPERA_NEGACAO,

    INST_COMPARA_MENOR,
    INST_COMPARA_MAIOR,
    INST_COMPARA_IGUAL,
    INST_COMPARA_DESIGUAL,
    INST_COMPARA_MENOR_IGUAL,
    INST_COMPARA_MAIOR_IGUAL,

    INST_DESVIO_SEMPRE,
    INST_DESVIO_FALSO,
    INST_DESVIO_ROTULO,
    
    INST_CONTROLE_NADA,
    INST_CONTROLE_PARA,
    INST_CONTROLE_LEITURA,
    INST_CONTROLE_IMPRIME,
    INST_CONTROLE_ALOCA_MEM,
    INST_CONTROLE_DELOCA_MEM,
    INST_CONTROLE_INICIA_PROGRAMA,
    INST_CONTROLE_ENTRAR_ROTULO,

    INST_CONTROLE_CHAMADA_PROCEDIMENTO,
    INST_CONTROLE_ENTRADA_PROCEDIMENTO,
    INST_CONTROLE_RETORNA_PROCEDIMENTO,

    XINTERNAL_LIMITE_TABELA_INSTRUCOES
} IndiceInstrucao;

typedef struct {
    int linha;
    int coluna;
} CompilationPosition;

extern string codigoInstrucao[XINTERNAL_LIMITE_TABELA_INSTRUCOES];
void writeCommand(IndiceInstrucao indice, string parametros[], int n);
void writeLabel(string label);

extern string caminhoArquivoCompilando;
extern FILE* arquivoCompilando;
extern CompilationPosition currentPosition;
extern char currentLine[CAP_SIZE_LINHA_BUFFER];

void readLineToEndError(FILE* currentFile);

extern string caminhoArquivoInstrucoes;
extern FILE* arquivoInstrucoes;
extern int escopoAtual;
extern int deslocamentoAtual;

#endif
