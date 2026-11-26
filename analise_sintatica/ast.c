#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* =========================================================
   FUNCOES AUXILIARES
   ========================================================= */

static char *copiaString(const char *texto) {
    if (texto == NULL) {
        return NULL;
    }

    size_t tamanho = strlen(texto) + 1;
    char *copia = malloc(tamanho);

    if (copia != NULL) {
        memcpy(copia, texto, tamanho);
    }

    return copia;
}

static void imprimeIndentacao(int nivel) {
    for (int i = 0; i < nivel; i++) {
        printf("    ");
    }
}

/* =========================================================
   EXPRESSOES
   ========================================================= */

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

    exp->dados.identificador = copiaString(nome);

    if (exp->dados.identificador == NULL) {
        free(exp);
        return NULL;
    }

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

/* =========================================================
   ARGUMENTOS E CHAMADAS DE FUNCAO
   ========================================================= */

Arg *criaArg(Exp *expressao) {
    Arg *arg = malloc(sizeof(Arg));

    if (arg == NULL) {
        return NULL;
    }

    arg->expressao = expressao;
    arg->prox = NULL;

    return arg;
}

void liberaArgs(Arg *args) {
    while (args != NULL) {
        Arg *proximo = args->prox;

        liberaExp(args->expressao);
        free(args);

        args = proximo;
    }
}

Exp *criaChamada(const char *nome, Arg *argumentos, int numArgs) {
    Exp *exp = malloc(sizeof(Exp));

    if (exp == NULL) {
        return NULL;
    }

    exp->dados.chamada.nome = copiaString(nome);

    if (exp->dados.chamada.nome == NULL) {
        free(exp);
        return NULL;
    }

    exp->tipo = NO_CHAMADA;
    exp->dados.chamada.argumentos = argumentos;
    exp->dados.chamada.numArgs = numArgs;

    return exp;
}

/* =========================================================
   LIBERACAO E IMPRESSAO DE EXPRESSOES
   ========================================================= */

void liberaExp(Exp *exp) {
    if (exp == NULL) {
        return;
    }

    switch (exp->tipo) {
        case NO_CONST:
            break;

        case NO_IDENT:
            free(exp->dados.identificador);
            break;

        case NO_OPBIN:
            liberaExp(exp->dados.opbin.esq);
            liberaExp(exp->dados.opbin.dir);
            break;

        case NO_CHAMADA:
            free(exp->dados.chamada.nome);
            liberaArgs(exp->dados.chamada.argumentos);
            break;
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

    switch (exp->tipo) {
        case NO_CONST:
            printf("%d", exp->dados.valor);
            break;

        case NO_IDENT:
            printf("%s", exp->dados.identificador);
            break;

        case NO_OPBIN:
            printf("(");
            imprimeExp(exp->dados.opbin.esq);
            printf(" %s ", operadorParaStr(exp->dados.opbin.op));
            imprimeExp(exp->dados.opbin.dir);
            printf(")");
            break;

        case NO_CHAMADA: {
            printf("%s(", exp->dados.chamada.nome);

            Arg *arg = exp->dados.chamada.argumentos;

            while (arg != NULL) {
                imprimeExp(arg->expressao);

                if (arg->prox != NULL) {
                    printf(", ");
                }

                arg = arg->prox;
            }

            printf(")");
            break;
        }
    }
}

/* =========================================================
   PARAMETROS
   ========================================================= */

Param *criaParam(const char *nome) {
    Param *param = malloc(sizeof(Param));

    if (param == NULL) {
        return NULL;
    }

    param->nome = copiaString(nome);

    if (param->nome == NULL) {
        free(param);
        return NULL;
    }

    param->prox = NULL;

    return param;
}

void liberaParams(Param *params) {
    while (params != NULL) {
        Param *proximo = params->prox;

        free(params->nome);
        free(params);

        params = proximo;
    }
}

/* =========================================================
   DECLARACOES DE VARIAVEIS
   ========================================================= */

VarDecl *criaVarDecl(const char *nome, Exp *expressao) {
    VarDecl *varDecl = malloc(sizeof(VarDecl));

    if (varDecl == NULL) {
        return NULL;
    }

    varDecl->identificador = copiaString(nome);

    if (varDecl->identificador == NULL) {
        free(varDecl);
        return NULL;
    }

    varDecl->expressao = expressao;
    varDecl->prox = NULL;

    return varDecl;
}

void liberaVarDecl(VarDecl *varDecl) {
    while (varDecl != NULL) {
        VarDecl *proximo = varDecl->prox;

        free(varDecl->identificador);
        liberaExp(varDecl->expressao);
        free(varDecl);

        varDecl = proximo;
    }
}

void imprimeVarDecl(VarDecl *varDecl) {
    while (varDecl != NULL) {
        printf("var %s = ", varDecl->identificador);
        imprimeExp(varDecl->expressao);
        printf(";\n");

        varDecl = varDecl->prox;
    }
}

/* =========================================================
   COMANDOS
   ========================================================= */

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

static void imprimeListaCmd(Cmd *cmd, int nivel) {
    while (cmd != NULL) {
        imprimeIndentacao(nivel);

        switch (cmd->tipo) {
            case CMD_IF:
                printf("if ");
                imprimeExp(cmd->dados.cmdIf.condicao);
                printf(" {\n");

                imprimeListaCmd(cmd->dados.cmdIf.blocoIf, nivel + 1);

                imprimeIndentacao(nivel);
                printf("} else {\n");

                imprimeListaCmd(cmd->dados.cmdIf.blocoElse, nivel + 1);

                imprimeIndentacao(nivel);
                printf("}\n");
                break;

            case CMD_WHILE:
                printf("while ");
                imprimeExp(cmd->dados.cmdWhile.condicao);
                printf(" {\n");

                imprimeListaCmd(cmd->dados.cmdWhile.corpo, nivel + 1);

                imprimeIndentacao(nivel);
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

/* =========================================================
   DECLARACOES DE FUNCOES
   ========================================================= */

FunDecl *criaFunDecl(const char *nome, Param *parametros, int numParams,
                     VarDecl *locais, int numLocais, Cmd *comandos,
                     Exp *expressaoFinal) {
    FunDecl *funDecl = malloc(sizeof(FunDecl));

    if (funDecl == NULL) {
        return NULL;
    }

    funDecl->nome = copiaString(nome);

    if (funDecl->nome == NULL) {
        free(funDecl);
        return NULL;
    }

    funDecl->parametros = parametros;
    funDecl->numParams = numParams;
    funDecl->locais = locais;
    funDecl->numLocais = numLocais;
    funDecl->comandos = comandos;
    funDecl->expressaoFinal = expressaoFinal;

    return funDecl;
}

void liberaFunDecl(FunDecl *funDecl) {
    if (funDecl == NULL) {
        return;
    }

    free(funDecl->nome);
    liberaParams(funDecl->parametros);
    liberaVarDecl(funDecl->locais);
    liberaCmd(funDecl->comandos);
    liberaExp(funDecl->expressaoFinal);

    free(funDecl);
}

void imprimeFunDecl(FunDecl *funDecl) {
    if (funDecl == NULL) {
        return;
    }

    printf("fun %s(", funDecl->nome);

    Param *param = funDecl->parametros;

    while (param != NULL) {
        printf("%s", param->nome);

        if (param->prox != NULL) {
            printf(", ");
        }

        param = param->prox;
    }

    printf(") {\n");

    imprimeVarDecl(funDecl->locais);

    imprimeListaCmd(funDecl->comandos, 1);

    printf("    return ");
    imprimeExp(funDecl->expressaoFinal);
    printf(";\n}\n");
}

/* =========================================================
   DECLARACOES DO PROGRAMA
   ========================================================= */

Decl *criaDeclVar(VarDecl *varDecl) {
    Decl *decl = malloc(sizeof(Decl));

    if (decl == NULL) {
        return NULL;
    }

    decl->tipo = DECL_VAR;
    decl->dados.varDecl = varDecl;
    decl->prox = NULL;

    return decl;
}

Decl *criaDeclFun(FunDecl *funDecl) {
    Decl *decl = malloc(sizeof(Decl));

    if (decl == NULL) {
        return NULL;
    }

    decl->tipo = DECL_FUN;
    decl->dados.funDecl = funDecl;
    decl->prox = NULL;

    return decl;
}

void liberaDecl(Decl *decl) {
    while (decl != NULL) {
        Decl *proximo = decl->prox;

        switch (decl->tipo) {
            case DECL_VAR:
                liberaVarDecl(decl->dados.varDecl);
                break;

            case DECL_FUN:
                liberaFunDecl(decl->dados.funDecl);
                break;
        }

        free(decl);
        decl = proximo;
    }
}

void imprimeDecl(Decl *decl) {
    while (decl != NULL) {
        switch (decl->tipo) {
            case DECL_VAR:
                imprimeVarDecl(decl->dados.varDecl);
                break;

            case DECL_FUN:
                imprimeFunDecl(decl->dados.funDecl);
                break;
        }

        decl = decl->prox;
    }
}

/* =========================================================
   PROGRAMA
   ========================================================= */

Program *criaProgram(Decl *declaracoes, Cmd *comandos,
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

    liberaDecl(programa->declaracoes);
    liberaCmd(programa->comandos);
    liberaExp(programa->expressaoFinal);

    free(programa);
}

void imprimeProgram(Program *programa) {
    if (programa == NULL) {
        return;
    }

    imprimeDecl(programa->declaracoes);

    printf("main {\n");

    imprimeListaCmd(programa->comandos, 1);

    printf("    return ");
    imprimeExp(programa->expressaoFinal);
    printf(";\n}\n");
}
