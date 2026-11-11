#ifndef AST_H
#define AST_H

typedef enum {
	OP_SOMA,
	OP_SUB,
	OP_MULT,
	OP_DIV
} Operador;

typedef enum {
    NO_CONST,
    NO_OPBIN,
    NO_IDENT
} TipoNo;

typedef struct Exp {
	TipoNo tipo;
	union {
		int valor; // usado quando tipo == NO_CONST
		char *identificador;
		struct {
			Operador op;
			struct Exp *esq;
			struct Exp *dir;
		} opbin; // usado quando tipo == NO_OPBIN
	} dados;
} Exp;

typedef struct Declaration {
    char *identificador;
    Exp *expressao;
    struct Declaration *prox;
} Declaration;

typedef struct Program {
    Declaration *declaracoes;
    Exp *expressaoFinal;
} Program;

Exp *criaConst(int valor);
Exp *criaOpBin(Operador op, Exp *esq, Exp *dir);
Exp *criaIdentificador(const char *nome);
void liberaExp(Exp *exp);
void imprimeExp(Exp *exp);
char operadorParaChar(Operador op);

Declaration *criaDeclaration(const char *nome, Exp *expressao);
void liberaDeclaration(Declaration *declaracao);
void imprimeDeclaration(Declaration *declaracao);

Program *criaProgram(Declaration *declaracoes, Exp *expressaoFinal);
void liberaProgram(Program *programa);
void imprimeProgram(Program *programa);

#endif
