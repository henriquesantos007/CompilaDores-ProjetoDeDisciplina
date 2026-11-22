#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

static void erroSintatico(Parser *parser, const char *mensagem) {
    fprintf(stderr, "Erro sintatico na posicao %zu: %s (token \"%s\")\n",
            parser->atual.indice,
            mensagem,
            parser->atual.lexema != NULL ? parser->atual.lexema : "");
    exit(1);
}

static void verificaErroLexico(Parser *parser) {
    if (parser->atual.tipo == ERRO) {
        erroSintatico(parser, "caractere nao reconhecido pelo analisador lexico");
    }
}

static void avanca(Parser *parser) {
    free(parser->atual.lexema);
    parser->atual = proximoToken(parser->lexer);
}

static void exigeToken(Parser *parser, TipoToken tipo, const char *mensagem) {
    verificaErroLexico(parser);

    if (parser->atual.tipo != tipo) {
        erroSintatico(parser, mensagem);
    }

    avanca(parser);
}

static char *copiaNome(const char *nome) {
    char *copia = malloc(strlen(nome) + 1);

    if (copia == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    strcpy(copia, nome);
    return copia;
}

Parser inicializaParser(Lexer *lexer) {
    Parser parser;
    parser.lexer = lexer;
    parser.atual = proximoToken(lexer);
    return parser;
}

Operador analisaOperador(Parser *parser) {
    verificaErroLexico(parser);

    Operador op;

    if (parser->atual.tipo == OP) {
        switch (parser->atual.lexema[0]) {
            case '+': op = OP_SOMA; break;
            case '-': op = OP_SUB; break;
            case '*': op = OP_MULT; break;
            case '/': op = OP_DIV; break;
            default:
                erroSintatico(parser, "operador aritmetico desconhecido");
                return OP_SOMA;
        }
    } else {
        switch (parser->atual.tipo) {
            case MENOR: op = OP_MENOR; break;
            case MAIOR: op = OP_MAIOR; break;
            case IGUAL_IGUAL: op = OP_IGUAL; break;
            default:
                erroSintatico(parser, "operador desconhecido");
                return OP_SOMA;
        }
    }

    avanca(parser);
    return op;
}

static Declaration *analisaDeclaracao(Parser *parser) {
    verificaErroLexico(parser);

    if (parser->atual.tipo != IDENTIFICADOR) {
        erroSintatico(parser, "esperado um identificador na declaracao");
    }

    char *nome = copiaNome(parser->atual.lexema);
    avanca(parser);

    exigeToken(parser, IGUAL, "= esperado na declaracao");

    Exp *expressao = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA, "; esperado apos a declaracao");

    Declaration *declaracao = criaDeclaration(nome, expressao);
    free(nome);

    if (declaracao == NULL) {
        liberaExp(expressao);
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    return declaracao;
}

static Exp *analisaPrim(Parser *parser) {
    verificaErroLexico(parser);

    if (parser->atual.tipo == NUM) {
        int valor = atoi(parser->atual.lexema);
        avanca(parser);

        Exp *exp = criaConst(valor);

        if (exp == NULL) {
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        return exp;
    }

    if (parser->atual.tipo == IDENTIFICADOR) {
        Exp *exp = criaIdentificador(parser->atual.lexema);

        if (exp == NULL) {
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        avanca(parser);
        return exp;
    }

    if (parser->atual.tipo == ABRE_PAR) {
        avanca(parser);

        Exp *expressao = analisaExpressao(parser);

        exigeToken(parser, FECHA_PAR, "esperado ')' para fechar a expressao");

        return expressao;
    }

    erroSintatico(parser, "esperado numero, identificador ou '('");
    return NULL;
}

static Exp *analisaExpM(Parser *parser) {
    Exp *esq = analisaPrim(parser);

    while (parser->atual.tipo == OP &&
           (parser->atual.lexema[0] == '*' ||
            parser->atual.lexema[0] == '/')) {

        Operador op = analisaOperador(parser);
        Exp *dir = analisaPrim(parser);
        Exp *nova = criaOpBin(op, esq, dir);

        if (nova == NULL) {
            liberaExp(esq);
            liberaExp(dir);
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        esq = nova;
    }

    return esq;
}

static Exp *analisaExpSoma(Parser *parser) {
    Exp *esq = analisaExpM(parser);

    while (parser->atual.tipo == OP &&
           (parser->atual.lexema[0] == '+' ||
            parser->atual.lexema[0] == '-')) {

        Operador op = analisaOperador(parser);
        Exp *dir = analisaExpM(parser);
        Exp *nova = criaOpBin(op, esq, dir);

        if (nova == NULL) {
            liberaExp(esq);
            liberaExp(dir);
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        esq = nova;
    }

    return esq;
}

Exp *analisaExpressao(Parser *parser) {
    Exp *esq = analisaExpSoma(parser);

    while (parser->atual.tipo == MENOR ||
           parser->atual.tipo == MAIOR ||
           parser->atual.tipo == IGUAL_IGUAL) {

        Operador op = analisaOperador(parser);
        Exp *dir = analisaExpSoma(parser);
        Exp *nova = criaOpBin(op, esq, dir);

        if (nova == NULL) {
            liberaExp(esq);
            liberaExp(dir);
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        esq = nova;
    }

    return esq;
}

static Cmd *analisaListaComandos(Parser *parser, TipoToken terminador);

static Cmd *analisaBloco(Parser *parser) {
    exigeToken(parser, ABRE_CHAVE, "esperado '{' para iniciar o bloco");

    Cmd *comandos = analisaListaComandos(parser, FECHA_CHAVE);

    exigeToken(parser, FECHA_CHAVE, "esperado '}' para fechar o bloco");

    return comandos;
}

static Cmd *analisaAtribuicao(Parser *parser) {
    verificaErroLexico(parser);

    if (parser->atual.tipo != IDENTIFICADOR) {
        erroSintatico(parser, "esperado identificador na atribuicao");
    }

    char *nome = copiaNome(parser->atual.lexema);
    avanca(parser);

    exigeToken(parser, IGUAL, "esperado '=' na atribuicao");

    Exp *expressao = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA, "esperado ';' apos a atribuicao");

    Cmd *cmd = criaCmdAtrib(nome, expressao);
    free(nome);

    if (cmd == NULL) {
        liberaExp(expressao);
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    return cmd;
}

static Cmd *analisaIf(Parser *parser) {
    avanca(parser);

    Exp *condicao = analisaExpressao(parser);
    Cmd *blocoIf = analisaBloco(parser);

    exigeToken(parser, KW_ELSE, "esperado 'else' apos o bloco do if");

    Cmd *blocoElse = analisaBloco(parser);

    Cmd *cmd = criaCmdIf(condicao, blocoIf, blocoElse);

    if (cmd == NULL) {
        liberaExp(condicao);
        liberaCmd(blocoIf);
        liberaCmd(blocoElse);
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    return cmd;
}

static Cmd *analisaWhile(Parser *parser) {
    avanca(parser);

    Exp *condicao = analisaExpressao(parser);
    Cmd *corpo = analisaBloco(parser);

    Cmd *cmd = criaCmdWhile(condicao, corpo);

    if (cmd == NULL) {
        liberaExp(condicao);
        liberaCmd(corpo);
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    return cmd;
}

static Cmd *analisaComando(Parser *parser) {
    verificaErroLexico(parser);

    switch (parser->atual.tipo) {
        case KW_IF:
            return analisaIf(parser);

        case KW_WHILE:
            return analisaWhile(parser);

        case IDENTIFICADOR:
            return analisaAtribuicao(parser);

        default:
            erroSintatico(parser, "comando nao reconhecido");
            return NULL;
    }
}

static Cmd *analisaListaComandos(Parser *parser, TipoToken terminador) {
    Cmd *inicio = NULL;
    Cmd *ultimo = NULL;

    while (parser->atual.tipo != terminador) {
        verificaErroLexico(parser);

        if (parser->atual.tipo == TOKEN_EOF) {
            liberaCmd(inicio);
            erroSintatico(parser, "fim inesperado do programa dentro do bloco");
        }

        Cmd *novo = analisaComando(parser);

        if (inicio == NULL) {
            inicio = novo;
        } else {
            ultimo->prox = novo;
        }

        ultimo = novo;
    }

    return inicio;
}

Program *analisaPrograma(Parser *parser) {
    Declaration *declaracoes = NULL;
    Declaration *ultima = NULL;

    while (parser->atual.tipo == IDENTIFICADOR) {
        Declaration *nova = analisaDeclaracao(parser);

        if (declaracoes == NULL) {
            declaracoes = nova;
        } else {
            ultima->prox = nova;
        }

        ultima = nova;
    }

    exigeToken(parser, ABRE_CHAVE, "esperado '{' para iniciar o programa");

    Cmd *comandos = analisaListaComandos(parser, KW_RETURN);

    exigeToken(parser, KW_RETURN, "esperado 'return' no final do bloco");

    Exp *expressaoFinal = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA, "esperado ';' apos a expressao do return");
    exigeToken(parser, FECHA_CHAVE, "esperado '}' para fechar o programa");

    verificaErroLexico(parser);

    if (parser->atual.tipo != TOKEN_EOF) {
        erroSintatico(parser, "tokens inesperados apos o fim do programa");
    }

    Program *programa = criaProgram(declaracoes, comandos, expressaoFinal);

    if (programa == NULL) {
        liberaDeclaration(declaracoes);
        liberaCmd(comandos);
        liberaExp(expressaoFinal);
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    return programa;
}