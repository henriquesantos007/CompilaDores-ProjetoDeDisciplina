#ifndef AST_H
#define AST_H

typedef enum {
    // Operadores aritméticos
    OP_SOMA,
    OP_SUB,
    OP_MULT,
    OP_DIV,

    // Novos operadores de comparação
    OP_MENOR,   // <
    OP_MAIOR,   // >
    OP_IGUAL    // ==
} Operador;

typedef enum {
    NO_CONST,
    NO_OPBIN,
    NO_IDENT
} TipoNo;

typedef struct Exp {
    TipoNo tipo;
    union {
        int valor;           // usado quando tipo == NO_CONST
        char *identificador; // usado quando tipo == NO_IDENT
        struct {
            Operador op;
            struct Exp *esq;
            struct Exp *dir;
        } opbin;             // usado quando tipo == NO_OPBIN
    } dados;
} Exp;

typedef struct Declaration {
    char *identificador;
    Exp *expressao;
    struct Declaration *prox;
} Declaration;

// As novas estruturas de comandos

typedef enum {
    CMD_IF,
    CMD_WHILE,
    CMD_ATRIB
} TipoCmd;

typedef struct Cmd {
    TipoCmd tipo;
    union {
        struct {
            Exp *condicao;
            struct Cmd *blocoIf;
            struct Cmd *blocoElse;
        } cmdIf;
        struct {
            Exp *condicao;
            struct Cmd *corpo;
        } cmdWhile;
        struct {
            char *identificador;
            Exp *expressao;
        } cmdAtrib;
    } dados;
    struct Cmd *prox;
} Cmd;

typedef struct Program {
    Declaration *declaracoes;
    Cmd *comandos;       // Lista de comandos dentro do bloco { ... }
    Exp *expressaoFinal; // Expressão do 'return <exp>;'
} Program;

// Funções de Expressão
Exp *criaConst(int valor);
Exp *criaOpBin(Operador op, Exp *esq, Exp *dir);
Exp *criaIdentificador(const char *nome);
void liberaExp(Exp *exp);
void imprimeExp(Exp *exp);
const char *operadorParaStr(Operador op); // Substitui char por string para suportar "==" que tem 2 caractres

// Funções de Declaração
Declaration *criaDeclaration(const char *nome, Exp *expressao);
void liberaDeclaration(Declaration *declaracao);
void imprimeDeclaration(Declaration *declaracao);

// Funções de Comando
Cmd *criaCmdIf(Exp *condicao, Cmd *blocoIf, Cmd *blocoElse);
Cmd *criaCmdWhile(Exp *condicao, Cmd *corpo);
Cmd *criaCmdAtrib(const char *nome, Exp *expressao);
void liberaCmd(Cmd *cmd);
void imprimeCmd(Cmd *cmd);

// Funções de Programa
Program *criaProgram(Declaration *declaracoes, Cmd *comandos, Exp *expressaoFinal);
void liberaProgram(Program *programa);
void imprimeProgram(Program *programa);

#endif