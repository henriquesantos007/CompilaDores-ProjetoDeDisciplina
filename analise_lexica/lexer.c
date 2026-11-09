#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

Lexer inicializaLexer(const char *buffer) {
	Lexer lexer;
	lexer.buffer = buffer;
	lexer.indice = 0;
	return lexer;
}

// Funções auxiliares

int fimBuffer(const Lexer *lexer) {
	return lexer->buffer[lexer->indice] == '\0';
}

void ignorarEspaco(Lexer *lexer) {
	while (!fimBuffer(lexer) &&
		(lexer->buffer[lexer->indice] == ' ' ||
		 lexer->buffer[lexer->indice] == '\n' ||
		 lexer->buffer[lexer->indice] == '\t')) {
		lexer->indice++;
	}
}

Token lerNumero(Lexer *lexer) {
    size_t inicio = (size_t)lexer->indice;

    while (!fimBuffer(lexer) &&
        isdigit((unsigned char)lexer->buffer[lexer->indice])) {
        lexer->indice++;
    }

    // Números seguidos de letras formam uma sequência inválida.
    if (!fimBuffer(lexer) &&
        isalpha((unsigned char)lexer->buffer[lexer->indice])) {

        while (!fimBuffer(lexer) &&
            (isalnum((unsigned char)lexer->buffer[lexer->indice]))) {
            lexer->indice++;
        }

        size_t tamanho = (size_t)lexer->indice - inicio;
        Token token = {ERRO, malloc(tamanho + 1), inicio};

        if (token.lexema == NULL) {
            return token;
        }

        memcpy(token.lexema, lexer->buffer + inicio, tamanho);
        token.lexema[tamanho] = '\0';

        return token;
    }

    size_t tamanho = (size_t)lexer->indice - inicio;
    Token token = {NUM, malloc(tamanho + 1), inicio};

    if (token.lexema == NULL) {
        token.tipo = ERRO;
        return token;
    }

    memcpy(token.lexema, lexer->buffer + inicio, tamanho);
    token.lexema[tamanho] = '\0';

    return token;
}

// Função para reconhecer identificadores (nomes de variáveis).
// Um identificador começa com uma letra e pode ser seguido por letras ou dígitos.
Token lerIdentificador(Lexer *lexer) {
    size_t inicio = (size_t)lexer->indice;

    // Continua lendo enquanto encontrar letras ou dígitos.
    while (!fimBuffer(lexer) &&
        (isalpha((unsigned char)lexer->buffer[lexer->indice]) ||
         isdigit((unsigned char)lexer->buffer[lexer->indice]))) {
        lexer->indice++;
    }

    // Calcula o tamanho do identificador encontrado.
    size_t tamanho = (size_t)lexer->indice - inicio;

    // Cria o token e aloca espaço para armazenar seu lexema.
    Token token = {IDENTIFICADOR, malloc(tamanho + 1), inicio};

    if (token.lexema == NULL) {
        token.tipo = ERRO;
        return token;
    }

    // Copia o identificador do código-fonte para o lexema.
    memcpy(token.lexema, lexer->buffer + inicio, tamanho);
    token.lexema[tamanho] = '\0';

    return token;
}

// Função de resgate dos tokens
Token proximoToken(Lexer *lexer) {
	ignorarEspaco(lexer); // Ignora espaços ou caracteres irrelevantes para a compilação
	size_t indice = (size_t)lexer->indice;
	TipoToken tipo;

    // Retornar Token de erro caso seja o fim do buffer
	if (fimBuffer(lexer)) {
		Token token = {TOKEN_EOF, malloc(1), indice};
		if (token.lexema != NULL) {
			token.lexema[0] = '\0';
		} else {
			token.tipo = ERRO;
		}
		return token;
	}


	char caractere = lexer->buffer[lexer->indice];

    if (isdigit((unsigned char)caractere)) {
    return lerNumero(lexer);
}

if (isalpha((unsigned char)caractere)) {
    return lerIdentificador(lexer);
}

// Segue para definir outras opções de tipos de tokens
switch (caractere) {
    case '(':
        tipo = ABRE_PAR;
        break;
    case ')':
        tipo = FECHA_PAR;
        break;
    case ';':
        tipo = PONTO_VIRGULA;
        break;
    case '=':
        tipo = IGUAL;
        break;
    case '+':
    case '-':
    case '/':
    case '*':
        tipo = OP;
        break;
    default: {
        Token token = {ERRO, malloc(2), indice};
        lexer->indice++;
        if (token.lexema != NULL) {
            token.lexema[0] = caractere;
            token.lexema[1] = '\0';
        }
        return token;
    }
}
    // Estruturação do retorno do token
	Token token = {tipo, malloc(2), indice};
	lexer->indice++;
	if (token.lexema == NULL) {
		token.tipo = ERRO;
		return token;
	}
	token.lexema[0] = caractere;
	token.lexema[1] = '\0';
    
	return token;
}