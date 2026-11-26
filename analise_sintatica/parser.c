#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

/* =========================================================
   FUNCOES AUXILIARES
   ========================================================= */

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

static void verificaAlocacao(void *ponteiro) {
    if (ponteiro == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }
}

/* =========================================================
   INICIALIZACAO DO PARSER
   ========================================================= */

Parser inicializaParser(Lexer *lexer) {
    Parser parser;
    parser.lexer = lexer;
    parser.atual = proximoToken(lexer);
    return parser;
}

/* =========================================================
   OPERADORES
   ========================================================= */

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

/* =========================================================
   EXPRESSOES
   ========================================================= */

Exp *analisaExpressao(Parser *parser);

static Arg *analisaListaArgumentos(Parser *parser, int *quantidade) {
    Arg *inicio = NULL;
    Arg *ultimo = NULL;

    *quantidade = 0;

    if (parser->atual.tipo == FECHA_PAR) {
        return NULL;
    }

    do {
        Exp *expressao = analisaExpressao(parser);
        Arg *novo = criaArg(expressao);

        if (novo == NULL) {
            liberaExp(expressao);
            liberaArgs(inicio);
            verificaAlocacao(NULL);
        }

        if (inicio == NULL) {
            inicio = novo;
        } else {
            ultimo->prox = novo;
        }

        ultimo = novo;
        (*quantidade)++;

        if (parser->atual.tipo != VIRGULA) {
            break;
        }

        avanca(parser);

    } while (1);

    return inicio;
}

static Exp *analisaPrim(Parser *parser) {
    verificaErroLexico(parser);

    if (parser->atual.tipo == NUM) {
        int valor = atoi(parser->atual.lexema);
        avanca(parser);

        Exp *exp = criaConst(valor);
        verificaAlocacao(exp);

        return exp;
    }

    if (parser->atual.tipo == IDENTIFICADOR) {
        char *nome = copiaNome(parser->atual.lexema);
        avanca(parser);

        if (parser->atual.tipo == ABRE_PAR) {
            avanca(parser);

            int quantidade;
            Arg *argumentos = analisaListaArgumentos(parser, &quantidade);

            exigeToken(parser, FECHA_PAR,
                       "esperado ')' apos os argumentos da chamada");

            Exp *chamada = criaChamada(nome, argumentos, quantidade);
            free(nome);

            if (chamada == NULL) {
                liberaArgs(argumentos);
                verificaAlocacao(NULL);
            }

            return chamada;
        }

        Exp *identificador = criaIdentificador(nome);
        free(nome);

        verificaAlocacao(identificador);
        return identificador;
    }

    if (parser->atual.tipo == ABRE_PAR) {
        avanca(parser);

        Exp *expressao = analisaExpressao(parser);

        exigeToken(parser, FECHA_PAR,
                   "esperado ')' para fechar a expressao");

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
            verificaAlocacao(NULL);
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
            verificaAlocacao(NULL);
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
            verificaAlocacao(NULL);
        }

        esq = nova;
    }

    return esq;
}

/* =========================================================
   DECLARACOES DE VARIAVEIS
   ========================================================= */

static VarDecl *analisaVarDecl(Parser *parser) {
    exigeToken(parser, KW_VAR, "esperado 'var'");

    if (parser->atual.tipo != IDENTIFICADOR) {
        erroSintatico(parser, "esperado identificador apos 'var'");
    }

    char *nome = copiaNome(parser->atual.lexema);
    avanca(parser);

    exigeToken(parser, IGUAL, "esperado '=' na declaracao");

    Exp *expressao = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA,
               "esperado ';' apos a declaracao");

    VarDecl *declaracao = criaVarDecl(nome, expressao);
    free(nome);

    if (declaracao == NULL) {
        liberaExp(expressao);
        verificaAlocacao(NULL);
    }

    return declaracao;
}

/* =========================================================
   LISTA DE PARAMETROS FORMAIS
   ========================================================= */

static Param *analisaParametros(Parser *parser, int *quantidade) {
    Param *inicio = NULL;
    Param *ultimo = NULL;

    *quantidade = 0;

    if (parser->atual.tipo == FECHA_PAR) {
        return NULL;
    }

    while (1) {
        if (parser->atual.tipo != IDENTIFICADOR) {
            liberaParams(inicio);
            erroSintatico(parser, "esperado identificador de parametro");
        }

        char *nome = copiaNome(parser->atual.lexema);
        avanca(parser);

        Param *novo = criaParam(nome);
        free(nome);

        if (novo == NULL) {
            liberaParams(inicio);
            verificaAlocacao(NULL);
        }

        if (inicio == NULL) {
            inicio = novo;
        } else {
            ultimo->prox = novo;
        }

        ultimo = novo;
        (*quantidade)++;

        if (parser->atual.tipo != VIRGULA) {
            break;
        }

        avanca(parser);
    }

    return inicio;
}

/* =========================================================
   LISTAS DE DECLARACOES LOCAIS
   ========================================================= */

static VarDecl *analisaListaVariaveis(Parser *parser, int *quantidade) {
    VarDecl *inicio = NULL;
    VarDecl *ultimo = NULL;

    *quantidade = 0;

    while (parser->atual.tipo == KW_VAR) {
        VarDecl *nova = analisaVarDecl(parser);

        if (inicio == NULL) {
            inicio = nova;
        } else {
            ultimo->prox = nova;
        }

        ultimo = nova;
        (*quantidade)++;
    }

    return inicio;
}

/* =========================================================
   DECLARACAO DE FUNCOES
   ========================================================= */
static Cmd *analisaListaComandos(Parser *parser, TipoToken terminador);
static FunDecl *analisaFunDecl(Parser *parser) {
    exigeToken(parser, KW_FUN, "esperado 'fun'");

    if (parser->atual.tipo != IDENTIFICADOR) {
        erroSintatico(parser, "esperado nome da funcao");
    }

    char *nome = copiaNome(parser->atual.lexema);
    avanca(parser);

    exigeToken(parser, ABRE_PAR, "esperado '(' apos nome da funcao");

    int numParams;
    Param *parametros = analisaParametros(parser, &numParams);

    exigeToken(parser, FECHA_PAR, "esperado ')' apos os parametros");

    exigeToken(parser, ABRE_CHAVE, "esperado '{' no corpo da funcao");

    int numLocais;
    VarDecl *locais = analisaListaVariaveis(parser, &numLocais);

    Cmd *comandos = analisaListaComandos(parser, KW_RETURN);

    exigeToken(parser, KW_RETURN, "esperado 'return' na funcao");

    Exp *expressaoFinal = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA,
               "esperado ';' apos o return da funcao");

    exigeToken(parser, FECHA_CHAVE,
               "esperado '}' para fechar a funcao");

    FunDecl *funcao = criaFunDecl(
        nome,
        parametros,
        numParams,
        locais,
        numLocais,
        comandos,
        expressaoFinal
    );

    free(nome);

    if (funcao == NULL) {
        liberaParams(parametros);
        liberaVarDecl(locais);
        liberaCmd(comandos);
        liberaExp(expressaoFinal);
        verificaAlocacao(NULL);
    }

    return funcao;
}

/* =========================================================
   COMANDOS
   ========================================================= */

static Cmd *analisaListaComandos(Parser *parser, TipoToken terminador);

static Cmd *analisaBloco(Parser *parser) {
    exigeToken(parser, ABRE_CHAVE, "esperado '{' para iniciar o bloco");

    Cmd *comandos = analisaListaComandos(parser, FECHA_CHAVE);

    exigeToken(parser, FECHA_CHAVE, "esperado '}' para fechar o bloco");

    return comandos;
}

static Cmd *analisaAtribuicao(Parser *parser) {
    if (parser->atual.tipo != IDENTIFICADOR) {
        erroSintatico(parser, "esperado identificador na atribuicao");
    }

    char *nome = copiaNome(parser->atual.lexema);
    avanca(parser);

    exigeToken(parser, IGUAL, "esperado '=' na atribuicao");

    Exp *expressao = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA,
               "esperado ';' apos a atribuicao");

    Cmd *cmd = criaCmdAtrib(nome, expressao);
    free(nome);

    if (cmd == NULL) {
        liberaExp(expressao);
        verificaAlocacao(NULL);
    }

    return cmd;
}

static Cmd *analisaIf(Parser *parser) {
    exigeToken(parser, KW_IF, "esperado 'if'");

    Exp *condicao = analisaExpressao(parser);
    Cmd *blocoIf = analisaBloco(parser);

    exigeToken(parser, KW_ELSE, "esperado 'else' apos o bloco do if");

    Cmd *blocoElse = analisaBloco(parser);

    Cmd *cmd = criaCmdIf(condicao, blocoIf, blocoElse);

    if (cmd == NULL) {
        liberaExp(condicao);
        liberaCmd(blocoIf);
        liberaCmd(blocoElse);
        verificaAlocacao(NULL);
    }

    return cmd;
}

static Cmd *analisaWhile(Parser *parser) {
    exigeToken(parser, KW_WHILE, "esperado 'while'");

    Exp *condicao = analisaExpressao(parser);
    Cmd *corpo = analisaBloco(parser);

    Cmd *cmd = criaCmdWhile(condicao, corpo);

    if (cmd == NULL) {
        liberaExp(condicao);
        liberaCmd(corpo);
        verificaAlocacao(NULL);
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

/* =========================================================
   PROGRAMA
   ========================================================= */

Program *analisaPrograma(Parser *parser) {
    Decl *declaracoes = NULL;
    Decl *ultima = NULL;

    /* Lê as declarações globais e funções */
    while (parser->atual.tipo == KW_VAR ||
           parser->atual.tipo == KW_FUN) {

        Decl *nova = NULL;

        if (parser->atual.tipo == KW_VAR) {
            VarDecl *varDecl = analisaVarDecl(parser);
            nova = criaDeclVar(varDecl);

            if (nova == NULL) {
                liberaVarDecl(varDecl);
                verificaAlocacao(NULL);
            }
        } else {
            FunDecl *funDecl = analisaFunDecl(parser);
            nova = criaDeclFun(funDecl);

            if (nova == NULL) {
                liberaFunDecl(funDecl);
                verificaAlocacao(NULL);
            }
        }

        if (declaracoes == NULL) {
            declaracoes = nova;
        } else {
            ultima->prox = nova;
        }

        ultima = nova;
    }

    /* Início do bloco main */
    exigeToken(parser, KW_MAIN,
               "esperado 'main' apos as declaracoes");

    exigeToken(parser, ABRE_CHAVE,
               "esperado '{' apos 'main'");

    Cmd *comandos = analisaListaComandos(parser, KW_RETURN);

    exigeToken(parser, KW_RETURN,
               "esperado 'return' no bloco main");

    Exp *expressaoFinal = analisaExpressao(parser);

    exigeToken(parser, PONTO_VIRGULA,
               "esperado ';' apos expressao do return");

    exigeToken(parser, FECHA_CHAVE,
               "esperado '}' para fechar main");

    /* Verifica se não existem tokens depois do programa */
    verificaErroLexico(parser);

    if (parser->atual.tipo != TOKEN_EOF) {
        liberaDecl(declaracoes);
        liberaCmd(comandos);
        liberaExp(expressaoFinal);

        erroSintatico(parser,
                      "tokens inesperados apos o fim do programa");
    }

    Program *programa = criaProgram(
        declaracoes,
        comandos,
        expressaoFinal
    );

    if (programa == NULL) {
        liberaDecl(declaracoes);
        liberaCmd(comandos);
        liberaExp(expressaoFinal);
        verificaAlocacao(NULL);
    }

    return programa;
}