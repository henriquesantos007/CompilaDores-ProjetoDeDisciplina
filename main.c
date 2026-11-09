#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "gerador.h"

const char *nomeTipoToken(TipoToken tipo) {
    switch (tipo) {
        case ABRE_PAR:
            return "ABRE_PAR";
        case FECHA_PAR:
            return "FECHA_PAR";
        case NUM:
            return "NUM";
        case OP:
            return "OP";
        case PONTO_VIRGULA:
            return "PONTO_VIRGULA";
        case IGUAL:
            return "IGUAL";
        case IDENTIFICADOR:
            return "IDENTIFICADOR";
        case TOKEN_EOF:
            return "TOKEN_EOF";
        case ERRO:
            return "ERRO";
        default:
            return "DESCONHECIDO";
    }
}

int main( int argc, char *argv[]){
    int modoAssembly = argc == 3 && strcmp(argv[1], "--asm") == 0;
    const char *caminhoEntrada = modoAssembly ? argv[2] :
        (argc == 2 ? argv[1] : NULL);

    //Verifica a quantidade de argumentos
    if (caminhoEntrada == NULL) {
    fprintf(stderr, "Uso: %s [--asm] <arquivo>\n", argv[0]);
    return 1;
    }

    //Abre o arquivo
    FILE *arquivo = fopen(caminhoEntrada, "r");
    if (arquivo == NULL) {
    perror("Erro ao abrir o arquivo");
    return 1;
    }

    //verifica o tamanho do arquivo 
    if (fseek(arquivo, 0, SEEK_END) != 0) {
    perror("Erro ao buscar o fim do arquivo");
    fclose(arquivo);
    return 1;
    }
    long tamanho = ftell(arquivo);
    if (tamanho < 0 || fseek(arquivo, 0, SEEK_SET) != 0) {
    perror("Erro ao determinar o tamanho do arquivo");
    fclose(arquivo);
    return 1;
    }

    //reserva memória para o conteúdo do arquivo
    char *buffer = malloc((size_t)tamanho + 1);
    if (buffer == NULL) {
    fprintf(stderr, "Erro ao alocar memória.\n");
    fclose(arquivo);
    return 1;
    }

    //Ler o conteúdo do arquivo
    size_t lidos = fread(buffer, sizeof(char), (size_t)tamanho, arquivo);
    buffer[lidos] = '\0';
    if (lidos != (size_t)tamanho || ferror(arquivo)) {
    fprintf(stderr, "Erro ao ler o arquivo de entrada.\n");
    free(buffer);
    fclose(arquivo);
    return 1;
    }

    Lexer lexer = inicializaLexer(buffer);

    Token token;

    do {
        token = proximoToken(&lexer);

        if (token.tipo == ERRO) {
            fprintf(stderr,
                "Erro lexico na posicao %zu: caractere nao reconhecido \"%s\"\n",
                token.indice,
                token.lexema != NULL ? token.lexema : "?");
        }

        if (!modoAssembly) {
            printf("<%s, \"%s\", %zu>\n",
                nomeTipoToken(token.tipo),
                token.lexema != NULL ? token.lexema : "",
                token.indice);
        }

        free(token.lexema);

    } while (token.tipo != TOKEN_EOF && token.tipo != ERRO);

    int codigoSaida = 0;

    if (token.tipo == ERRO) {
        codigoSaida = 1;
    }

    // Analise sintatica: refaz a leitura do buffer com um novo Lexer, ja que
    // o primeiro foi consumido inteiramente durante a impressao dos tokens acima
    if (codigoSaida == 0) {
        Lexer lexerSintatico = inicializaLexer(buffer);
        Parser parser = inicializaParser(&lexerSintatico);

        Exp *arvore = analisaPrograma(&parser);

        if (modoAssembly) {
            if (gerar_codigo_ast(arvore, stdout) != 0) {
                fprintf(stderr, "Erro ao gerar o codigo assembly.\n");
                codigoSaida = 1;
            }
        } else {
            printf("Arvore Sintatica: ");
            imprimeExp(arvore);
            printf("\n");
        }

        liberaExp(arvore);
        free(parser.atual.lexema);
    }

    fclose(arquivo);
    free(buffer);

    return codigoSaida;
}