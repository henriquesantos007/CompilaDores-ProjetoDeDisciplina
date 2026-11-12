#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "gerador.h"
#include "analise_semantica.h"



// Estruturas e Funções Auxiliares

typedef struct {
    const char *caminhoEntrada;
    bool modoAssembly;
} ConfiguracaoCli;

static const char *nomeTipoToken(TipoToken tipo) {
    switch (tipo) {
        case ABRE_PAR:      return "ABRE_PAR";
        case FECHA_PAR:     return "FECHA_PAR";
        case NUM:           return "NUM";
        case OP:            return "OP";
        case PONTO_VIRGULA: return "PONTO_VIRGULA";
        case IGUAL:         return "IGUAL";
        case IDENTIFICADOR: return "IDENTIFICADOR";
        case TOKEN_EOF:     return "TOKEN_EOF";
        case ERRO:          return "ERRO";
        default:            return "DESCONHECIDO";
    }
}

// Validar e extrair os argumentos passados pelo terminal
static ConfiguracaoCli analisarArgumentos(int argc, char *argv[]) {
    ConfiguracaoCli config = {NULL, false};
    
    if (argc == 3 && strcmp(argv[1], "--asm") == 0) {
        config.modoAssembly = true;
        config.caminhoEntrada = argv[2];
    } else if (argc == 2) {
        config.caminhoEntrada = argv[1];
    }
    
    return config;
}

// Isolar toda a lógica de I/O de disco e alocação do buffer
static char* lerConteudoArquivo(const char *caminho) {
    FILE *arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo); // Volta o cursor para o início

    if (tamanho < 0) {
        perror("Erro ao determinar o tamanho do arquivo");
        fclose(arquivo);
        return NULL;
    }

    char *buffer = malloc((size_t)tamanho + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Erro ao alocar memoria para leitura do arquivo.\n");
        fclose(arquivo);
        return NULL;
    }

    size_t lidos = fread(buffer, sizeof(char), (size_t)tamanho, arquivo);
    buffer[lidos] = '\0';

    if (lidos != (size_t)tamanho || ferror(arquivo)) {
        fprintf(stderr, "Erro ao ler o arquivo de entrada.\n");
        free(buffer);
        fclose(arquivo);
        return NULL;
    }

    fclose(arquivo);
    return buffer;
}

// Rodar o analisador léxico para exibir logs e caçar erros primários
static int executarFaseLexica(const char *buffer, bool modoAssembly) {
    Lexer lexer = inicializaLexer(buffer);
    Token token;
    int codigoErro = 0;

    do {
        token = proximoToken(&lexer);

        if (token.tipo == ERRO) {
            fprintf(stderr, "Erro lexico na posicao %zu: caractere nao reconhecido \"%s\"\n",
                    token.indice, token.lexema != NULL ? token.lexema : "?");
            codigoErro = 1;
        }

        // Imprime os tokens na tela apenas se não estiver no modo de geração de assembly
        if (!modoAssembly && token.tipo != ERRO && token.tipo != TOKEN_EOF) {
            printf("<%s, \"%s\", %zu>\n",
                   nomeTipoToken(token.tipo),
                   token.lexema != NULL ? token.lexema : "",
                   token.indice);
        }

        free(token.lexema);
    } while (token.tipo != TOKEN_EOF && token.tipo != ERRO);

    return codigoErro;
}

// Orquestrar a Sintaxe, Semântica e Síntese de forma sequencial
static int executarPipelineCompilacao(const char *buffer, bool modoAssembly) {
    Lexer lexer = inicializaLexer(buffer);
    Parser parser = inicializaParser(&lexer);

    // Fase Sintática pra gerar a AST
    Program *programa = analisaPrograma(&parser);
    int codigoSaida = 0;

    // Fase Semântica que valida variáveis fantasmas na AST com a Hash Table
    analisarSemantica(programa);

    // Fase de Síntese onde gera o Assembly ou imprime a Árvore
    if (modoAssembly) {
        if (gerar_codigo_ast(programa, stdout) != 0) {
            fprintf(stderr, "Erro ao gerar o codigo assembly.\n");
            codigoSaida = 1;
        }
    } else {
        printf("Arvore Sintatica:\n");
        imprimeProgram(programa);
    }

    // Limpeza de memória do pipeline
    liberaProgram(programa);
    free(parser.atual.lexema);
    
    return codigoSaida;
}


int main(int argc, char *argv[]) {
    // Configuração
    ConfiguracaoCli config = analisarArgumentos(argc, argv);
    if (config.caminhoEntrada == NULL) {
        fprintf(stderr, "Uso: %s [--asm] <arquivo>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Leitura
    char *buffer = lerConteudoArquivo(config.caminhoEntrada);
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }

    // Fase Léxica de Validação
    int status = executarFaseLexica(buffer, config.modoAssembly);

    // Compilação (só entra se o léxico não encontrar erros)
    if (status == 0) {
        status = executarPipelineCompilacao(buffer, config.modoAssembly);
    }

    free(buffer);
    return status;
}