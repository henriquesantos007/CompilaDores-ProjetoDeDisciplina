#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

Exp *criaConst(int valor) {
    Exp *exp = malloc(sizeof(Exp));

    if (exp == NULL) {
        return NULL;
    }

    exp->tipo = NO_CONST;
    exp->dados.valor = valor;

    return exp;
}

Exp *criaIdentificador(const char *nome) {
    Exp *exp = malloc(sizeof(Exp));

    if (exp == NULL) {
        return NULL;
    }

    exp->dados.identificador = malloc(strlen(nome) + 1);

    if (exp->dados.identificador == NULL) {
        free(exp);
        return NULL;
    }

    memcpy(exp->dados.identificador, nome, strlen(nome) + 1);
    exp->tipo = NO_IDENT;

    return exp;
}

Exp *criaOpBin(Operador op, Exp *esq, Exp *dir) {
    Exp *exp = malloc(sizeof(Exp));

    if (exp == NULL) {
        return NULL;
    }

    exp->tipo = NO_OPBIN;
    exp->dados.opbin.op = op;
    exp->dados.opbin.esq = esq;
    exp->dados.opbin.dir = dir;

    return exp;
}

void liberaExp(Exp *exp) {
    if (exp == NULL) {
        return;
    }

    if (exp->tipo == NO_OPBIN) {
        liberaExp(exp->dados.opbin.esq);
        liberaExp(exp->dados.opbin.dir);
    } else if (exp->tipo == NO_IDENT) {
        free(exp->dados.identificador);
    }

    free(exp);
}

const char *operadorParaStr(Operador op) {
    switch (op) {
        case OP_SOMA: return "+";
        case OP_SUB: return "-";
        case OP_MULT: return "*";
        case OP_DIV: return "/";
        case OP_MENOR: return "<";
        case OP_MAIOR: return ">";
        case OP_IGUAL: return "==";
        default: return "?";
    }
}

void imprimeExp(Exp *exp) {
    if (exp == NULL) {
        return;
    }

    if (exp->tipo == NO_CONST) {
        printf("%d", exp->dados.valor);
    } else if (exp->tipo == NO_IDENT) {
        printf("%s", exp->dados.identificador);
    } else if (exp->tipo == NO_OPBIN) {
        printf("(");
        imprimeExp(exp->dados.opbin.esq);
        printf(" %s ", operadorParaStr(exp->dados.opbin.op));
        imprimeExp(exp->dados.opbin.dir);
        printf(")");
    }
}

Declaration *criaDeclaration(const char *nome, Exp *expressao) {
    Declaration *declaracao = malloc(sizeof(Declaration));

    if (declaracao == NULL) {
        return NULL;
    }

    declaracao->identificador = malloc(strlen(nome) + 1);

    if (declaracao->identificador == NULL) {
        free(declaracao);
        return NULL;
    }

    memcpy(declaracao->identificador, nome, strlen(nome) + 1);
    declaracao->expressao = expressao;
    declaracao->prox = NULL;

    return declaracao;
}

void liberaDeclaration(Declaration *declaracao) {
    while (declaracao != NULL) {
        Declaration *proxima = declaracao->prox;

        free(declaracao->identificador);
        liberaExp(declaracao->expressao);
        free(declaracao);

        declaracao = proxima;
    }
}

void imprimeDeclaration(Declaration *declaracao) {
    while (declaracao != NULL) {
        printf("%s = ", declaracao->identificador);
        imprimeExp(declaracao->expressao);
        printf(";\n");

        declaracao = declaracao->prox;
    }
}

static char *copiaString(const char *texto) {
    char *copia = malloc(strlen(texto) + 1);

    if (copia != NULL) {
        memcpy(copia, texto, strlen(texto) + 1);
    }

    return copia;
}

Cmd *criaCmdIf(Exp *condicao, Cmd *blocoIf, Cmd *blocoElse) {
    Cmd *cmd = malloc(sizeof(Cmd));

    if (cmd == NULL) {
        return NULL;
    }

    cmd->tipo = CMD_IF;
    cmd->dados.cmdIf.condicao = condicao;
    cmd->dados.cmdIf.blocoIf = blocoIf;
    cmd->dados.cmdIf.blocoElse = blocoElse;
    cmd->prox = NULL;

    return cmd;
}

Cmd *criaCmdWhile(Exp *condicao, Cmd *corpo) {
    Cmd *cmd = malloc(sizeof(Cmd));

    if (cmd == NULL) {
        return NULL;
    }

    cmd->tipo = CMD_WHILE;
    cmd->dados.cmdWhile.condicao = condicao;
    cmd->dados.cmdWhile.corpo = corpo;
    cmd->prox = NULL;

    return cmd;
}

Cmd *criaCmdAtrib(const char *nome, Exp *expressao) {
    Cmd *cmd = malloc(sizeof(Cmd));

    if (cmd == NULL) {
        return NULL;
    }

    cmd->dados.cmdAtrib.identificador = copiaString(nome);

    if (cmd->dados.cmdAtrib.identificador == NULL) {
        free(cmd);
        return NULL;
    }

    cmd->tipo = CMD_ATRIB;
    cmd->dados.cmdAtrib.expressao = expressao;
    cmd->prox = NULL;

    return cmd;
}

void liberaCmd(Cmd *cmd) {
    while (cmd != NULL) {
        Cmd *proximo = cmd->prox;

        switch (cmd->tipo) {
            case CMD_IF:
                liberaExp(cmd->dados.cmdIf.condicao);
                liberaCmd(cmd->dados.cmdIf.blocoIf);
                liberaCmd(cmd->dados.cmdIf.blocoElse);
                break;

            case CMD_WHILE:
                liberaExp(cmd->dados.cmdWhile.condicao);
                liberaCmd(cmd->dados.cmdWhile.corpo);
                break;

            case CMD_ATRIB:
                free(cmd->dados.cmdAtrib.identificador);
                liberaExp(cmd->dados.cmdAtrib.expressao);
                break;
        }

        free(cmd);
        cmd = proximo;
    }
}

static void imprimeComIndentacao(int nivel) {
    for (int i = 0; i < nivel; i++) {
        printf("    ");
    }
}

static void imprimeListaCmd(Cmd *cmd, int nivel) {
    while (cmd != NULL) {
        imprimeComIndentacao(nivel);

        switch (cmd->tipo) {
            case CMD_IF:
                printf("if ");
                imprimeExp(cmd->dados.cmdIf.condicao);
                printf(" {\n");

                imprimeListaCmd(cmd->dados.cmdIf.blocoIf, nivel + 1);

                imprimeComIndentacao(nivel);
                printf("} else {\n");

                imprimeListaCmd(cmd->dados.cmdIf.blocoElse, nivel + 1);

                imprimeComIndentacao(nivel);
                printf("}\n");
                break;

            case CMD_WHILE:
                printf("while ");
                imprimeExp(cmd->dados.cmdWhile.condicao);
                printf(" {\n");

                imprimeListaCmd(cmd->dados.cmdWhile.corpo, nivel + 1);

                imprimeComIndentacao(nivel);
                printf("}\n");
                break;

            case CMD_ATRIB:
                printf("%s = ", cmd->dados.cmdAtrib.identificador);
                imprimeExp(cmd->dados.cmdAtrib.expressao);
                printf(";\n");
                break;
        }

        cmd = cmd->prox;
    }
}

void imprimeCmd(Cmd *cmd) {
    imprimeListaCmd(cmd, 0);
}

Program *criaProgram(Declaration *declaracoes, Cmd *comandos,
                     Exp *expressaoFinal) {
    Program *programa = malloc(sizeof(Program));

    if (programa == NULL) {
        return NULL;
    }

    programa->declaracoes = declaracoes;
    programa->comandos = comandos;
    programa->expressaoFinal = expressaoFinal;

    return programa;
}

void liberaProgram(Program *programa) {
    if (programa == NULL) {
        return;
    }

    liberaDeclaration(programa->declaracoes);
    liberaCmd(programa->comandos);
    liberaExp(programa->expressaoFinal);

    free(programa);
}

void imprimeProgram(Program *programa) {
    if (programa == NULL) {
        return;
    }

    imprimeDeclaration(programa->declaracoes);

    printf("{\n");
    imprimeListaCmd(programa->comandos, 1);

    printf("    return ");
    imprimeExp(programa->expressaoFinal);
    printf(";\n}\n");
}