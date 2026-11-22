#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

Lexer inicializaLexer(const char *entrada) {
    Lexer lexer;
    lexer.entrada = entrada;
    lexer.posicao = 0;
    return lexer;
}

static int fimEntrada(const Lexer *lexer) {
    return lexer->entrada[lexer->posicao] == '\0';
}

static void ignorarEspaco(Lexer *lexer) {
    while (!fimEntrada(lexer) &&
           isspace((unsigned char)lexer->entrada[lexer->posicao])) {
        lexer->posicao++;
    }
}

static Token criaToken(Lexer *lexer, TipoToken tipo, size_t inicio) {
    size_t tamanho = lexer->posicao - inicio;
    Token token = {tipo, malloc(tamanho + 1), inicio};

    if (token.lexema == NULL) {
        token.tipo = ERRO;
        return token;
    }

    memcpy(token.lexema, lexer->entrada + inicio, tamanho);
    token.lexema[tamanho] = '\0';

    return token;
}

static Token lerNumero(Lexer *lexer) {
    size_t inicio = lexer->posicao;

    while (!fimEntrada(lexer) &&
           isdigit((unsigned char)lexer->entrada[lexer->posicao])) {
        lexer->posicao++;
    }

    if (!fimEntrada(lexer) &&
        isalpha((unsigned char)lexer->entrada[lexer->posicao])) {

        while (!fimEntrada(lexer) &&
               isalnum((unsigned char)lexer->entrada[lexer->posicao])) {
            lexer->posicao++;
        }

        return criaToken(lexer, ERRO, inicio);
    }

    return criaToken(lexer, NUM, inicio);
}

static Token lerIdentificador(Lexer *lexer) {
    size_t inicio = lexer->posicao;

    while (!fimEntrada(lexer) &&
           (isalnum((unsigned char)lexer->entrada[lexer->posicao]))) {
        lexer->posicao++;
    }

    size_t tamanho = lexer->posicao - inicio;
    const char *nome = lexer->entrada + inicio;
    TipoToken tipo = IDENTIFICADOR;

    if (tamanho == 2 && strncmp(nome, "if", tamanho) == 0) {
        tipo = KW_IF;
    } else if (tamanho == 4 && strncmp(nome, "else", tamanho) == 0) {
        tipo = KW_ELSE;
    } else if (tamanho == 5 && strncmp(nome, "while", tamanho) == 0) {
        tipo = KW_WHILE;
    } else if (tamanho == 6 && strncmp(nome, "return", tamanho) == 0) {
        tipo = KW_RETURN;
    }

    return criaToken(lexer, tipo, inicio);
}

Token proximoToken(Lexer *lexer) {
    ignorarEspaco(lexer);

    size_t inicio = lexer->posicao;

    if (fimEntrada(lexer)) {
        return criaToken(lexer, TOKEN_EOF, inicio);
    }

    char caractere = lexer->entrada[lexer->posicao];

    if (isdigit((unsigned char)caractere)) {
        return lerNumero(lexer);
    }

    if (isalpha((unsigned char)caractere)) {
        return lerIdentificador(lexer);
    }

    lexer->posicao++;

    switch (caractere) {
        case '(':
            return criaToken(lexer, ABRE_PAR, inicio);

        case ')':
            return criaToken(lexer, FECHA_PAR, inicio);

        case '{':
            return criaToken(lexer, ABRE_CHAVE, inicio);

        case '}':
            return criaToken(lexer, FECHA_CHAVE, inicio);

        case ';':
            return criaToken(lexer, PONTO_VIRGULA, inicio);

        case '+':
        case '-':
        case '*':
        case '/':
            return criaToken(lexer, OP, inicio);

        case '<':
            return criaToken(lexer, MENOR, inicio);

        case '>':
            return criaToken(lexer, MAIOR, inicio);

        case '=':
            if (lexer->entrada[lexer->posicao] == '=') {
                lexer->posicao++;
                return criaToken(lexer, IGUAL_IGUAL, inicio);
            }
            return criaToken(lexer, IGUAL, inicio);

        default:
            return criaToken(lexer, ERRO, inicio);
    }
}

const char *tipoTokenParaStr(TipoToken tipo) {
    switch (tipo) {
        case IDENTIFICADOR: return "IDENTIFICADOR";
        case NUM: return "NUM";
        case OP: return "OP";
        case IGUAL: return "IGUAL";
        case PONTO_VIRGULA: return "PONTO_VIRGULA";
        case ABRE_PAR: return "ABRE_PAR";
        case FECHA_PAR: return "FECHA_PAR";
        case TOKEN_EOF: return "TOKEN_EOF";
        case ERRO: return "ERRO";
        case ABRE_CHAVE: return "ABRE_CHAVE";
        case FECHA_CHAVE: return "FECHA_CHAVE";
        case MENOR: return "MENOR";
        case MAIOR: return "MAIOR";
        case IGUAL_IGUAL: return "IGUAL_IGUAL";
        case KW_IF: return "KW_IF";
        case KW_ELSE: return "KW_ELSE";
        case KW_WHILE: return "KW_WHILE";
        case KW_RETURN: return "KW_RETURN";
        default: return "TOKEN_DESCONHECIDO";
    }
}