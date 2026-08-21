#include <stdlib.h>

#include "analisador/tokenlexico.h"
#include "basics/basics.h"

Token tokenDefinitions[] = {
    { TOKEN_KEYW_PROGRAMA,        "program",   false },
    { TOKEN_KEYW_ROTULO,          "label",     false },
    { TOKEN_KEYW_TIPO,            "type",      false },
    { TOKEN_KEYW_VARIAVEL,        "var",       false },
    { TOKEN_KEYW_PROCEDIMENTO,    "procedure", false },
    { TOKEN_KEYW_FUNCAO,          "function",  false },
    { TOKEN_KEYW_INICIO,          "begin",     false },
    { TOKEN_KEYW_FIM,             "end",       false },
    { TOKEN_KEYW_SE,              "if",        false },
    { TOKEN_KEYW_ENTAO,           "then",      false },
    { TOKEN_KEYW_SENAO,           "else",      false },
    { TOKEN_KEYW_ENQUANTO,        "while",     false },
    { TOKEN_KEYW_FACA,            "do",        false },
    { TOKEN_KEYW_VAPARA,          "goto",      false },

    { TOKEN_SYMB_ATRIBUICAO,      ":=",        true  },
    { TOKEN_SYMB_MAIS,            "+",         true  },
    { TOKEN_SYMB_MENOS,           "-",         true  },
    { TOKEN_SYMB_VEZES,           "*",         true  },
    { TOKEN_SYMB_DIVIDIR,         "/",         true  },

    { TOKEN_SYMB_IGUAL,           "=",         true  },
    { TOKEN_SYMB_DIFERENTE,       "<>",        true  },
    { TOKEN_SYMB_MENOR,           "<",         true  },
    { TOKEN_SYMB_MENORIGUAL,      "<=",        true  },
    { TOKEN_SYMB_MAIOR,           ">",         true  },
    { TOKEN_SYMB_MAIORIGUAL,      ">=",        true  },

    { TOKEN_SYMB_E,               "and",       false },
    { TOKEN_SYMB_OU,              "or",        false },
    { TOKEN_SYMB_NAO,             "not",       false },

    { TOKEN_SYMB_ABREPARENTESES,  "(",         true  },
    { TOKEN_SYMB_FECHAPARENTESES, ")",         true  },
    { TOKEN_SYMB_ABRECOLCHETES,   "[",         true  },
    { TOKEN_SYMB_FECHACOLCHETES,  "]",         true  },
    { TOKEN_SYMB_VIRGULA,         ",",         true  },
    { TOKEN_SYMB_PONTOVIRGULA,    ";",         true  },
    { TOKEN_SYMB_DOISPONTOS,      ":",         true  },
    { TOKEN_SYMB_PONTO,           ".",         true  },
    
    { TOKEN_SYMB_ABRECOMENTARIO,  "(*",        true  },
    { TOKEN_SYMB_FECHACOMENTARIO, "*)",        true  },
    
    { TOKEN_OPER_INVALIDO,        "invalido",  false },
    { TOKEN_OPER_EOF,             "eof",       false }
};
