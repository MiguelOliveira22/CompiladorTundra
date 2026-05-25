#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "analisadortools.h"
#include "error.h"
#include "basics.h"

const i8 MAX_LENGTH_LEXICO = 36;
const token tokens[] = {
    { programa,        "program",   false, "programa"        },
    { rotulo,          "label",     false, "rotulo"          },
    { tipo,            "type",      false, "tipo"            },
    { variavel,        "var",       false, "variavel"        },
    { procedimento,    "procedure", false, "procedimento"    },
    { funcao,          "function",  false, "funcao"          },
    { inicio,          "begin",     false, "inicio"          },
    { fim,             "end",       false, "fim"             },
    { atribuicao,      ":=",        true,  "atribuicao"      },
    { se,              "if",        false, "se"              },
    { entao,           "then",      false, "entao"           },
    { senao,           "else",      false, "senao"           },
    { enquanto,        "while",     false, "enquanto"        },
    { faca,            "do",        false, "faca"            },
    { vapara,          "goto",      false, "vapara"          },

    { mais,            "+",         true,  "mais"            },
    { menos,           "-",         true,  "menos"           },
    { vezes,           "*",         true,  "vezes"           },
    { dividir,         "/",         true,  "dividir"         },

    { igual,           "=",         true,  "igual"           },
    { diferente,       "<>",        true,  "diferente"       },
    { menor,           "<",         true,  "menor"           },
    { menorouigual,    "<=",        true,  "menorouigual"    },
    { maior,           ">",         true,  "maior"           },
    { maiorouigual,    ">=",        true,  "maiorouigual"    },

    { e,               "and",       false, "e"               },
    { ou,              "or",        false, "ou"              },
    { nao,             "not",       false, "nao"             },

    { abreparenteses,  "(",         true,  "abreparenteses"  },
    { fechaparenteses, ")",         true,  "fechaparenteses" },
    { abrecolchetes,   "[",         true,  "abrecolchetes"   },
    { fechacolchetes,  "]",         true,  "fechacolchetes"  },
    { virgula,         ",",         true,  "virgula"         },
    { pontoevirgula,   ";",         true,  "pontoevirgula"   },
    { doispontos,      ":",         true,  "doispontos"      },
    { ponto,           ".",         true,  "ponto"           },
    
    { abrecomentario,  "(*",        true,  "abrecomentario"  },
    { fechacomentario, "*)",        true,  "fechacomentario" },
    
    { invalido,        "",          false, "invalido"        },
    { eof,             "",          false, "EOF"             }
};
