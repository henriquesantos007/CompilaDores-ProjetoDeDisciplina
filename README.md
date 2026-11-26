# Atividade 09 — Compilador da Linguagem Fun (Funções e Escopo)

**Disciplina:** Construção de Compiladores  
**Plataforma alvo:** x86-64 / Linux (GNU Assembler)

Compilador para a linguagem **Fun**, que estende a linguagem Cmd (variáveis, `if`/`else`, `while`, atribuições e operadores relacionais) com declaração de funções, parâmetros, variáveis locais, chamadas (inclusive recursivas) e bloco principal `main`, gerando código Assembly x86-64 executável para Linux.

## 1. Grupo

| Nome                               | Matrícula   |
| ---------------------------------- | ----------- |
| Allan Gabriel da Cunha Vasconcelos | 20240009142 |
| Luiz Henrique Santos da Graça      | 20240008261 |
| Maria Eloiza Monteiro de Souza     | 20240008280 |
| Vinicius Victor Lopes de Almeida   | 20240008181 |

## 2. Etapa Léxica

O analisador léxico (`lexer.h`/`lexer.c`) lê o código-fonte de um arquivo e o converte em uma sequência de tokens, ignorando espaços, tabulações e quebras de linha. O lexer realiza *lookahead* de um caractere para diferenciar `=` de `==` e verifica se um identificador corresponde a uma palavra reservada antes de emitir `IDENTIFICADOR`.

| Token           | Lexemas reconhecidos |
| --------------- | -------------------- |
| `IDENTIFICADOR` | Nomes de variáveis e funções (ex: `x`, `soma`, `fib`) |
| `NUM`           | Números inteiros (ex: `321`) |
| `OP`            | Operadores aritméticos (`+`, `-`, `*`, `/`) |
| `MENOR`         | Operador relacional menor que (`<`) |
| `MAIOR`         | Operador relacional maior que (`>`) |
| `IGUAL_IGUAL`   | Operador relacional de igualdade (`==`) |
| `IGUAL`         | Atribuição e declaração (`=`) |
| `PONTO_VIRGULA` | Delimitador de fim de instrução (`;`) |
| `VIRGULA`       | Separador de parâmetros e argumentos (`,`) |
| `ABRE_PAR`      | `(` |
| `FECHA_PAR`     | `)` |
| `ABRE_CHAVE`    | Abertura de bloco (`{`) |
| `FECHA_CHAVE`   | Fechamento de bloco (`}`) |
| `KW_IF`         | Palavra-chave condicional (`if`) |
| `KW_ELSE`       | Palavra-chave alternativa (`else`) |
| `KW_WHILE`      | Palavra-chave de repetição (`while`) |
| `KW_RETURN`     | Palavra-chave de expressão final (`return`) |
| `KW_FUN`        | Declaração de função (`fun`) |
| `KW_VAR`        | Declaração de variável (`var`) |
| `KW_MAIN`       | Bloco principal do programa (`main`) |
| `TOKEN_EOF`     | Fim do arquivo |
| `ERRO`          | Caractere não reconhecido |

## 3. Etapa Sintática

O analisador sintático (`parser.h`/`parser.c`) constrói a Árvore Sintática Abstrata (`ast.h`/`ast.c`) por análise descendente recursiva. Na linguagem Fun, o programa é uma sequência de declarações de variáveis globais e funções seguida do bloco `main`. O `if` continua exigindo `else` e os operadores relacionais têm precedência menor que os aritméticos:

```text
<programa> ::= <decl>* 'main' '{' <cmd>* 'return' <exp> ';' '}'
<decl>     ::= <vardecl> | <fundecl>
<vardecl>  ::= 'var' <id> '=' <exp> ';'
<fundecl>  ::= 'fun' <id> '(' <arglist>? ')' '{' <vardecl>* <cmd>* 'return' <exp> ';' '}'
<arglist>  ::= <id> (',' <id>)*
<cmd>      ::= <if> | <while> | <atrib>
<if>       ::= 'if' <exp> '{' <cmd>* '}' 'else' '{' <cmd>* '}'
<while>    ::= 'while' <exp> '{' <cmd>* '}'
<atrib>    ::= <id> '=' <exp> ';'
<exp>      ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
<exp_a>    ::= <exp_m> (('+' | '-') <exp_m>)*
<exp_m>    ::= <prim> (('*' | '/') <prim>)*
<prim>     ::= <num> | <id> | <id> '(' <explist>? ')' | '(' <exp> ')'
<explist>  ::= <exp> (',' <exp>)*
<id>       ::= <letra><letra_digito>*
<num>      ::= <digito><digito>*
```

A estrutura `Program` guarda a lista ordenada de declarações (`Decl`, que pode ser `DECL_VAR` ou `DECL_FUN`), os comandos do `main` e a expressão final. Cada `FunDecl` guarda nome, parâmetros (`Param`), variáveis locais (`VarDecl`), comandos e expressão de retorno. Chamadas de função são representadas pelo nó `NO_CHAMADA`, com a lista de argumentos (`Arg`).

## 4. Etapa Semântica (Tabela de Símbolos)

A Análise Semântica (`analise_semantica.c` / `tabela_simbolos.c`) valida a AST antes da geração de código e devolve a tabela global preenchida para a síntese:
- **Hash Table:** algoritmo *djb2* com encadeamento separado. Cada símbolo guarda seu tipo (`SIMB_VAR_GLOBAL`, `SIMB_VAR_LOCAL` ou `SIMB_FUNCAO`); funções guardam número de parâmetros, número de locais e sua própria tabela local.
- **Escopos:** existe uma tabela global (variáveis globais e funções) e uma tabela local por função (parâmetros e variáveis locais). Dentro de uma função a busca é feita primeiro na tabela local e depois na global, permitindo *shadowing*.
- **Funções:** a função é registrada na tabela global antes da análise do corpo, permitindo recursão. Os offsets em relação a `%rbp` seguem a convenção: locais em `i * 8` e parâmetros em `(L + 2 + j) * 8`, onde `L` é o número de locais.
- **Chamadas (`NO_CHAMADA`):** o nome precisa existir, ser uma função e o número de argumentos precisa ser igual ao número de parâmetros.
- **Comandos e expressões:** variáveis usadas ou atribuídas precisam ter sido declaradas (no escopo local ou global) e não podem ser funções.
- Qualquer erro emite uma mensagem em `stderr` e encerra a compilação com `exit(1)`.

## 5. Etapa de Síntese (Geração de Código x86-64)

O motor de síntese (`gerador.c`) traduz a AST validada em um programa Assembly x86-64 organizado em duas seções:
- **Seção `.bss`:** Percorre as declarações e utiliza `.lcomm` para reservar 8 bytes em memória RAM (64 bits) para cada variável.
- **Seção `.text`:**
  - **Expressões aritméticas (pilha LIFO):** Constantes e variáveis são empilhadas (`pushq`), desempilhadas em `%rbx` e `%rax` (`popq`), processadas pela ULA (`addq`, `subq`, `imulq`, `cqto`/`idivq`) e empilhadas novamente.
  - **Operadores relacionais (`<`, `>`, `==`):** Utilizam a convenção `0 = falso` e `!= 0 = verdadeiro`. Comparam os operandos via `cmpq %rbx, %rax`, gravam o resultado dos sinalizadores (`RFLAGS`) em `%cl` com `setl`, `setg` ou `setz` (após zerar `%rcx` com `xorq`) e movem o valor para `%rax`.
  - **Controle de fluxo e atribuição:** `CMD_ATRIB` avalia a expressão e sobrescreve a variável na `.bss` com `movq`. `CMD_IF` e `CMD_WHILE` testam a condição com `cmpq $0, %rax` e orquestram saltos condicionais (`jz`) e incondicionais (`jmp`) usando um contador interno para gerar rótulos únicos (`LfalsoN`, `LfimN`, `LinicioN`).

## 6. Como Executar e Testar

### Pré-requisitos
Ambiente Linux (x86-64) com as seguintes ferramentas instaladas:
- `gcc` (Compilador C)
- `as` (GNU Assembler)
- `ld` (GNU Linker)

### 🤖 1. Execução Automatizada (Bateria de Testes)
O script `tests/test_sintese.sh` compila o motor em C, executa os exemplos válidos (`exemplo01.txt` a `exemplo12.txt`) conferindo a saída de cada binário gerado e verifica se os casos inválidos (`erro01.txt` a `erro08.txt`) são corretamente rejeitados pela análise semântica.

Na raiz do projeto, dê permissão de execução e rode o script:
```bash
chmod +x tests/test_sintese.sh
./tests/test_sintese.sh
```

### 🛠️ 2. Execução Manual Passo a Passo
Para compilar e executar manualmente qualquer código-fonte da linguagem Fun:

**Passo A: Compilar o Motor do Compilador (C)**
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

**Passo B: Visualizar Tokens e Árvore Sintática (Modo Padrão)**
Execute sem a flag `--asm` e sem redirecionar a saída:
```bash
./compilador tests/exemplo09.txt
```

**Passo C: Traduzir o código-fonte para Assembly x86-64 (`--asm`)**
Acrescente a flag `--asm` para imprimir exclusivamente o código Assembly e redirecione para um arquivo `.s`:
```bash
./compilador --asm tests/exemplo09.txt > programa.s
```

**Passo D: Montar, Linkar e Executar o Binário Final**
*(Execute na raiz do projeto para incluir corretamente o `runtime.s`)*
```bash
as --64 programa.s -o programa.o
ld programa.o -o programa
./programa
```

## 7. Arquivos de Exemplo e Testes

A pasta `tests/` contém programas de validação para a linguagem Fun:
- `tests/exemplo01.txt` a `tests/exemplo07.txt`: exemplos das atividades anteriores reescritos na sintaxe Fun (`var` e `main`).
- `tests/exemplo08.txt`: fatorial recursivo (função com parâmetro e variável local).
- `tests/exemplo09.txt`: Fibonacci recursivo.
- `tests/exemplo10.txt`: funções chamando outras funções, incluindo função sem parâmetros e sem locais.
- `tests/exemplo11.txt`: *shadowing* de variáveis globais por parâmetro e por variável local.
- `tests/exemplo12.txt`: MDC em função com `while`, chamada como argumento e função que altera variável global.
- `tests/erro01.txt` a `tests/erro03.txt`: uso/atribuição de variável não declarada no `main`.
- `tests/erro04.txt` a `tests/erro08.txt`: chamada a função não declarada, número errado de argumentos, variável chamada como função, uso de variável local de outra função e atribuição a variável não declarada dentro de função.
- `tests/test_sintese.sh`: script de validação automática *end-to-end*.

## 8. Organização da Arquitetura

O projeto adota o Princípio da Responsabilidade Única (SRP) com separação modular por fases do compilador:
- `analise_lexica/`: Implementação do Lexer e mapeamento de Tokens (`lexer.h`/`lexer.c`).
- `analise_sintatica/`: Parser descendente recursivo (`parser.h`/`parser.c`) e estruturas da AST (`ast.h`/`ast.c`).
- `analise_semantica/`: Validador de escopos, funções e comandos (`analise_semantica.c`) e Tabela de Símbolos em Hash Table (`tabela_simbolos.c`).
- `sintese/`: Gerador de código Assembly x86-64 (`gerador.h`/`gerador.c`).
- `runtime.s`: Rotinas de sistema (`imprime_num` e `sair`), incluídas no Assembly gerado.
- `main.c`: Orquestrador do pipeline de compilação e seleção de modo (`--asm` vs padrão).
- `tests/`: Exemplos de entrada válidos, casos de erro e script de testes automatizados.

## 9. Uso de LLMs

Nas atividades anteriores, o grupo utilizou o Google Gemini como apoio na engenharia da Tabela de Símbolos (*djb2*), depuração, testes, documentação e refatoração do `main.c` seguindo princípios de *Clean Code*. Para a **Atividade 08**, o grupo utilizou o **Claude (Anthropic)** como apoio na análise semântica dos comandos, nos arquivos de teste e no esboço da documentação, e o **Google Gemini** no planejamento da arquitetura *Contract-First*, na geração de código x86-64 para comandos/comparações, nas revisões de código e na consolidação deste `README.md`. Para a **Atividade 09**, o grupo utilizou o **Claude (Anthropic)** como apoio na análise semântica de funções e escopos, nos arquivos de teste e na atualização deste `README.md`. Todo o código gerado foi revisado, compilado e validado manualmente pelo grupo.