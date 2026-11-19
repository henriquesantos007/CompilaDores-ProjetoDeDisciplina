#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    IDENTIFICADOR,
    NUM,
    OP,             // +, -, *, /
    IGUAL,          // = (atribuição e declaração)
    PONTO_VIRGULA,  // ;
    ABRE_PAR,       // (
    FECHA_PAR,      // )
    TOKEN_EOF,
    ERRO,

    // Os Novos tokens pra essa Atividade 08
    ABRE_CHAVE,     // {
    FECHA_CHAVE,    // }
    MENOR,          // <
    MAIOR,          // >
    IGUAL_IGUAL,    // == (operador de igualdade)
    KW_IF,          // if
    KW_ELSE,        // else
    KW_WHILE,       // while
    KW_RETURN       // return
} TipoToken;

typedef struct {
    TipoToken tipo;
    char *lexema;
    size_t indice;
} Token;

typedef struct {
    const char *entrada;
    size_t posicao;
} Lexer;

Lexer inicializaLexer(const char *entrada);
Token proximoToken(Lexer *lexer);
const char *tipoTokenParaStr(TipoToken tipo);

#endif