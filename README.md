# Atividade 08 — Compilador da Linguagem Cmd

Compilador para a linguagem Cmd (declarações, `if`/`else`, `while`, atribuição e `return`), gerando Assembly x86-64 para Linux.

## 1. Como compilar

Na raiz do projeto (precisa de `gcc`, `as` e `ld`):

```bash
gcc -std=c11 -Wall -Wextra -Werror \
    -Ianalise_lexica -Ianalise_sintatica -Ianalise_semantica -Isintese \
    main.c \
    analise_lexica/lexer.c \
    analise_sintatica/parser.c analise_sintatica/ast.c \
    analise_semantica/analise_semantica.c analise_semantica/tabela_simbolos.c \
    sintese/gerador.c \
    -o compilador
```

## 2. Como executar

Modo padrão (mostra os tokens e a AST):

```bash
./compilador tests/exemplo05.txt
```

Modo `--asm` (gera o Assembly, monta, linka e executa):

```bash
./compilador --asm tests/exemplo05.txt > programa.s
as --64 programa.s -o programa.o
ld programa.o -o programa
./programa
```

## 3. Como rodar os testes

```bash
chmod +x tests/test_sintese.sh
./tests/test_sintese.sh
```

O script compila o compilador, roda os exemplos `tests/exemplo01.txt` a `tests/exemplo07.txt` conferindo o resultado de cada um e verifica se `tests/erro01.txt` a `tests/erro03.txt` são rejeitados (uso ou atribuição de variável não declarada).

## 4. Integrantes

| Nome                               | Matrícula   |
| ---------------------------------- | ----------- |
| Allan Gabriel da Cunha Vasconcelos | 20240009142 |
| Luiz Henrique Santos da Graça      | 20240008261 |
| Maria Eloiza Monteiro de Souza     | 20240008280 |
| Vinicius Victor Lopes de Almeida   | 20240008181 |

## 5. Uso de LLMs

Nas atividades anteriores o grupo usou o Google Gemini como apoio (tabela de símbolos, testes, README e refatoração do `main.c`). Na Atividade 08, o Claude (Anthropic) foi usado como apoio na análise semântica dos comandos, nos arquivos de teste e neste README. Todo o código foi revisado e testado pelo grupo.
