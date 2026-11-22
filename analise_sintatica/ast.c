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

Program *criaProgram(Declaration *declaracoes, Exp *expressaoFinal) {
    Program *programa = malloc(sizeof(Program));

    if (programa == NULL) {
        return NULL;
    }

    programa->declaracoes = declaracoes;
    programa->expressaoFinal = expressaoFinal;

    return programa;
}

void liberaProgram(Program *programa) {
    if (programa == NULL) {
        return;
    }

    liberaDeclaration(programa->declaracoes);
    liberaExp(programa->expressaoFinal);
    free(programa);
}

void imprimeProgram(Program *programa) {
    if (programa == NULL) {
        return;
    }

    imprimeDeclaration(programa->declaracoes);
    printf("= ");
    imprimeExp(programa->expressaoFinal);
    printf("\n");
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

char operadorParaChar(Operador op) {
	switch (op) {
		case OP_SOMA:
			return '+';
		case OP_SUB:
			return '-';
		case OP_MULT:
			return '*';
		case OP_DIV:
			return '/';
		default:
			return '?';
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
        printf(" %c ", operadorParaChar(exp->dados.opbin.op));
        imprimeExp(exp->dados.opbin.dir);
        printf(")");
    }
}
