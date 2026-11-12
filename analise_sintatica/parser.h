#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
	Lexer *lexer;
	Token atual; // lookahead de 1 token
} Parser;

Parser inicializaParser(Lexer *lexer);
Program *analisaPrograma(Parser *parser);
Exp *analisaExpressao(Parser *parser);
Operador analisaOperador(Parser *parser);

#endif
