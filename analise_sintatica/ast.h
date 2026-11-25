#ifndef AST_H
#define AST_H

typedef enum {
    // Operadores aritméticos
    OP_SOMA,
    OP_SUB,
    OP_MULT,
    OP_DIV,

    // Operadores de comparação
    OP_MENOR,   // <
    OP_MAIOR,   // >
    OP_IGUAL    // ==
} Operador;

typedef enum {
    NO_CONST,
    NO_OPBIN,
    NO_IDENT,
    NO_CHAMADA  // Chamada de função: f(arg1, arg2, ...)
} TipoNo;

typedef struct Exp Exp;

// Lista encadeada de argumentos reais (expressões) na chamada de função
typedef struct Arg {
    Exp *expressao;
    struct Arg *prox;
} Arg;

struct Exp {
    TipoNo tipo;
    union {
        int valor;           // NO_CONST
        char *identificador; // NO_IDENT
        struct {
            Operador op;
            Exp *esq;
            Exp *dir;
        } opbin;             // NO_OPBIN
        struct {
            char *nome;
            Arg *argumentos; // Lista de argumentos reais (pode ser NULL)
            int numArgs;
        } chamada;           // NO_CHAMADA
    } dados;
};

// Lista encadeada de parâmetros formais na declaração de função
typedef struct Param {
    char *nome;
    struct Param *prox;
} Param;

// Declaração de variável (usada tanto globalmente quanto localmente em funções)
typedef struct VarDecl {
    char *identificador;
    Exp *expressao;
    struct VarDecl *prox;
} VarDecl;

// Estruturas de comandos
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

// Declaração de função: fun <id>(<params>) { <locais> <comandos> return <exp>; }
typedef struct FunDecl {
    char *nome;
    Param *parametros;   // Lista de parâmetros formais (pode ser NULL)
    int numParams;
    VarDecl *locais;     // Lista de variáveis locais (pode ser NULL)
    int numLocais;
    Cmd *comandos;       // Lista de comandos do corpo (pode ser NULL)
    Exp *expressaoFinal; // Expressão do 'return <exp>;'
} FunDecl;

// Declaração de topo de programa (preserva a ordem entre 'var' e 'fun')
typedef enum {
    DECL_VAR,
    DECL_FUN
} TipoDecl;

typedef struct Decl {
    TipoDecl tipo;
    union {
        VarDecl *varDecl;
        FunDecl *funDecl;
    } dados;
    struct Decl *prox;
} Decl;

typedef struct Program {
    Decl *declaracoes;   // Sequência ordenada de variáveis globais e funções
    Cmd *comandos;       // Lista de comandos do bloco 'main { ... }'
    Exp *expressaoFinal; // Expressão do 'return <exp>;' do main
} Program;

// Funções de Expressão e Argumentos
Exp *criaConst(int valor);
Exp *criaOpBin(Operador op, Exp *esq, Exp *dir);
Exp *criaIdentificador(const char *nome);
Exp *criaChamada(const char *nome, Arg *argumentos, int numArgs);
Arg *criaArg(Exp *expressao);
void liberaExp(Exp *exp);
void liberaArgs(Arg *args);
void imprimeExp(Exp *exp);
const char *operadorParaStr(Operador op);

// Funções de Parâmetros e Declarações
Param *criaParam(const char *nome);
void liberaParams(Param *params);

VarDecl *criaVarDecl(const char *nome, Exp *expressao);
void liberaVarDecl(VarDecl *varDecl);
void imprimeVarDecl(VarDecl *varDecl);

FunDecl *criaFunDecl(const char *nome, Param *parametros, int numParams,
                     VarDecl *locais, int numLocais, Cmd *comandos,
                     Exp *expressaoFinal);
void liberaFunDecl(FunDecl *funDecl);
void imprimeFunDecl(FunDecl *funDecl);

Decl *criaDeclVar(VarDecl *varDecl);
Decl *criaDeclFun(FunDecl *funDecl);
void liberaDecl(Decl *decl);
void imprimeDecl(Decl *decl);

// Funções de Comando
Cmd *criaCmdIf(Exp *condicao, Cmd *blocoIf, Cmd *blocoElse);
Cmd *criaCmdWhile(Exp *condicao, Cmd *corpo);
Cmd *criaCmdAtrib(const char *nome, Exp *expressao);
void liberaCmd(Cmd *cmd);
void imprimeCmd(Cmd *cmd);

// Funções de Programa
Program *criaProgram(Decl *declaracoes, Cmd *comandos, Exp *expressaoFinal);
void liberaProgram(Program *programa);
void imprimeProgram(Program *programa);

#endif