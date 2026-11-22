#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Reporta um erro (léxico ou sintático) encontrado durante a análise e encerra o programa
static void erroSintatico(Parser *parser, const char *mensagem) {
	fprintf(stderr, "Erro sintatico na posicao %zu: %s (token \"%s\")\n",
		parser->atual.indice, mensagem, parser->atual.lexema);
	exit(1);
}

static void verificaErroLexico(Parser *parser) {
    if (parser->atual.tipo == ERRO) {
        erroSintatico(parser, "caractere nao reconhecido pelo analisador lexico");
    }
}

// Libera o token atual e busca o próximo token do lexer
static void avanca(Parser *parser) {
	free(parser->atual.lexema);
	parser->atual = proximoToken(parser->lexer);
}

Parser inicializaParser(Lexer *lexer) {
	Parser parser;
	parser.lexer = lexer;
	parser.atual = proximoToken(lexer);
	return parser;
}

Operador analisaOperador(Parser *parser) {
	if (parser->atual.tipo == ERRO) {
		erroSintatico(parser, "caractere nao reconhecido pelo analisador lexico");
	}
	if (parser->atual.tipo != OP) {
		erroSintatico(parser, "esperado um operador (+, -, * ou /)");
	}

	Operador op;
	switch (parser->atual.lexema[0]) {
		case '+':
			op = OP_SOMA;
			break;
		case '-':
			op = OP_SUB;
			break;
		case '*':
			op = OP_MULT;
			break;
		case '/':
			op = OP_DIV;
			break;
		default:
			erroSintatico(parser, "operador desconhecido");
			op = OP_SOMA; // inalcançável: erroSintatico encerra o programa
	}

	avanca(parser);
	return op;
}

static Declaration *analisaDeclaracao(Parser *parser) {
    verificaErroLexico(parser);

    if (parser->atual.tipo != IDENTIFICADOR) {
        erroSintatico(parser, "esperado um identificador");
    }

    char *nome = malloc(strlen(parser->atual.lexema) + 1);

    if (nome == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    strcpy(nome, parser->atual.lexema);
    avanca(parser);

    if (parser->atual.tipo != IGUAL) {
        erroSintatico(parser, "= esperado");
    }

    avanca(parser);

    Exp *expressao = analisaExpressao(parser);

    if (parser->atual.tipo != PONTO_VIRGULA) {
        erroSintatico(parser, "; esperado");
    }

    avanca(parser);

    Declaration *declaracao = criaDeclaration(nome, expressao);
    free(nome);

    if (declaracao == NULL) {
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
        return criaConst(valor);
    }

    if (parser->atual.tipo == IDENTIFICADOR) {
        Exp *identificador = criaIdentificador(parser->atual.lexema);

        if (identificador == NULL) {
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        avanca(parser);
        return identificador;
    }

    if (parser->atual.tipo == ABRE_PAR) {
        avanca(parser);

        Exp *expressao = analisaExpressao(parser);

        if (parser->atual.tipo != FECHA_PAR) {
            erroSintatico(parser, "esperado ')' para fechar a expressao");
        }

        avanca(parser);
        return expressao;
    }

    erroSintatico(parser, "esperado um numero, identificador ou '('");
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

Exp *analisaExpressao(Parser *parser) {
    Exp *esq = analisaExpM(parser);

    while (parser->atual.tipo == OP &&
           (parser->atual.lexema[0] == '+' ||
            parser->atual.lexema[0] == '-')) {

        Operador op = analisaOperador(parser);
        Exp *dir = analisaExpM(parser);

        Exp *nova = criaOpBin(op, esq, dir);

        if (nova == NULL) {
            fprintf(stderr, "Erro ao alocar memoria.\n");
            exit(1);
        }

        esq = nova;
    }

    return esq;
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

    verificaErroLexico(parser);

    if (parser->atual.tipo != IGUAL) {
        liberaDeclaration(declaracoes);
        erroSintatico(parser, "= esperado");
    }

    avanca(parser);

    Exp *expressaoFinal = analisaExpressao(parser);

    if (parser->atual.tipo != TOKEN_EOF) {
        liberaDeclaration(declaracoes);
        liberaExp(expressaoFinal);
        erroSintatico(parser, "tokens inesperados apos o fim do programa");
    }

    Program *programa = criaProgram(declaracoes, expressaoFinal);

    if (programa == NULL) {
        liberaDeclaration(declaracoes);
        liberaExp(expressaoFinal);
        fprintf(stderr, "Erro ao alocar memoria.\n");
        exit(1);
    }

    return programa;
}