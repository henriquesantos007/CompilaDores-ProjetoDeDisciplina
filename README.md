# Atividade 08 — Compilador da Linguagem Cmd (Comandos e Fluxo de Controle)

**Disciplina:** Construção de Compiladores  
**Plataforma alvo:** x86-64 / Linux (GNU Assembler)

Compilador completo para a linguagem Turing-completa **Cmd**, com suporte a declarações de variáveis, comandos condicionais (`if`/`else`), laços de repetição (`while`), atribuições, operadores relacionais e retorno (`return`), gerando código Assembly x86-64 executável para Linux.

## 1. Grupo

| Nome                               | Matrícula   |
| ---------------------------------- | ----------- |
| Allan Gabriel da Cunha Vasconcelos | 20240009142 |
| Luiz Henrique Santos da Graça      | 20240008261 |
| Maria Eloiza Monteiro de Souza     | 20240008280 |
| Vinicius Victor Lopes de Almeida   | 20240008181 |

## 2. Etapa Léxica

O analisador léxico (`lexer.h`/`lexer.c`) lê o código-fonte de um arquivo e o converte em uma sequência de tokens, ignorando espaços, tabulações e quebras de linha. Na linguagem Cmd, o lexer realiza *lookahead* de um caractere para diferenciar `=` de `==` e verifica se um identificador corresponde a uma palavra reservada antes de emitir `IDENTIFICADOR`.

| Token           | Lexemas reconhecidos |
| --------------- | -------------------- |
| `IDENTIFICADOR` | Nomes de variáveis (ex: `x`, `soma`, `delta`) |
| `NUM`           | Números inteiros (ex: `321`) |
| `OP`            | Operadores aritméticos (`+`, `-`, `*`, `/`) |
| `MENOR`         | Operador relacional menor que (`<`) |
| `MAIOR`         | Operador relacional maior que (`>`) |
| `IGUAL_IGUAL`   | Operador relacional de igualdade (`==`) |
| `IGUAL`         | Atribuição e declaração (`=`) |
| `PONTO_VIRGULA` | Delimitador de fim de instrução (`;`) |
| `ABRE_PAR`      | `(` |
| `FECHA_PAR`     | `)` |
| `ABRE_CHAVE`    | Abertura de bloco (`{`) |
| `FECHA_CHAVE`   | Fechamento de bloco (`}`) |
| `KW_IF`         | Palavra-chave condicional (`if`) |
| `KW_ELSE`       | Palavra-chave alternativa (`else`) |
| `KW_WHILE`      | Palavra-chave de repetição (`while`) |
| `KW_RETURN`     | Palavra-chave de expressão final (`return`) |
| `TOKEN_EOF`     | Fim do arquivo |
| `ERRO`          | Caractere não reconhecido |

## 3. Etapa Sintática

O analisador sintático (`parser.h`/`parser.c`) constrói a Árvore Sintática Abstrata (`ast.h`/`ast.c`) por análise descendente recursiva. A gramática da linguagem Cmd separa declarações iniciais do corpo de comandos (delimitado por chaves), exige o bloco `else` em comandos `if` e posiciona os operadores relacionais (`<`, `>`, `==`) em um nível de precedência inferior aos aritméticos:

```text
<programa> ::= <decl>* '{' <cmd>* 'return' <exp> ';' '}'
<decl>     ::= <id> '=' <exp> ';'
<cmd>      ::= <if> | <while> | <atrib>
<if>       ::= 'if' <exp> '{' <cmd>* '}' 'else' '{' <cmd>* '}'
<while>    ::= 'while' <exp> '{' <cmd>* '}'
<atrib>    ::= <id> '=' <exp> ';'
<exp>      ::= <exp_a> (('<' | '>' | '==') <exp_a>)*
<exp_a>    ::= <exp_m> (('+' | '-') <exp_m>)*
<exp_m>    ::= <prim> (('*' | '/') <prim>)*
<prim>     ::= <num> | <id> | '(' <exp> ')'
<id>       ::= <letra><letra_digito>*
<num>      ::= <digito><digito>*
```

A estrutura `Program` encapsula a lista encadeada de declarações (`Declaration`), a lista encadeada de comandos (`Cmd` — suportando `CMD_IF`, `CMD_WHILE` e `CMD_ATRIB`, inclusive em blocos vazios ou aninhados) e a expressão final (`return`).

## 4. Etapa Semântica (Tabela de Símbolos)

Atuando como camada de proteção antes da geração de código, a Análise Semântica (`analise_semantica.c` / `tabela_simbolos.c`) garante a integridade do escopo:
- **Hash Table:** Implementada com o algoritmo *djb2* e tratamento de colisões por encadeamento separado, garantindo buscas e inserções em tempo médio `O(1)`.
- **Validação de Declarações e Expressões:** Registra todas as variáveis declaradas no início do programa e impede o uso de variáveis não declaradas.
- **Validação de Comandos (`verificaComandos`):** Percorre recursivamente a lista de comandos e blocos aninhados:
  - `CMD_ATRIB`: valida as variáveis da expressão (lado direito) e exige que a variável atribuída (lado esquerdo) já exista na tabela de símbolos (atribuições não criam variáveis novas).
  - `CMD_IF` e `CMD_WHILE`: validam as expressões de condição e percorrem recursivamente `blocoIf`, `blocoElse` e `corpo`.
- Qualquer violação emite uma mensagem descritiva em `stderr` e encerra a compilação com `exit(1)`.

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
O script `tests/test_sintese.sh` compila o motor em C, executa os exemplos válidos (`exemplo01.txt` a `exemplo07.txt`) conferindo a saída de cada binário gerado e verifica se os casos inválidos (`erro01.txt` a `erro03.txt`) são corretamente rejeitados pela análise semântica.

Na raiz do projeto, dê permissão de execução e rode o script:
```bash
chmod +x tests/test_sintese.sh
./tests/test_sintese.sh
```

### 🛠️ 2. Execução Manual Passo a Passo
Para compilar e executar manualmente qualquer código-fonte da linguagem Cmd:

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
./compilador tests/exemplo05.txt
```

**Passo C: Traduzir o código-fonte para Assembly x86-64 (`--asm`)**
Acrescente a flag `--asm` para imprimir exclusivamente o código Assembly e redirecione para um arquivo `.s`:
```bash
./compilador --asm tests/exemplo05.txt > programa.s
```

**Passo D: Montar, Linkar e Executar o Binário Final**
*(Execute na raiz do projeto para incluir corretamente o `runtime.s`)*
```bash
as --64 programa.s -o programa.o
ld programa.o -o programa
./programa
```

## 7. Arquivos de Exemplo e Testes

A pasta `tests/` contém programas de validação para a linguagem Cmd:
- `tests/exemplo01.txt` a `tests/exemplo03.txt`: Programas básicos com declarações, operações aritméticas e `return`.
- `tests/exemplo04.txt`: Uso de condicional `if`/`else` (cálculo do valor absoluto do discriminante `delta`).
- `tests/exemplo05.txt`: Uso de laço `while` (soma dos inteiros de `n` até `m - 1`).
- `tests/exemplo06.txt`: Cálculo do resto da divisão por subtrações sucessivas usando `while` e comparação `>`.
- `tests/exemplo07.txt`: Cálculo do Máximo Divisor Comum (MDC) com laços `while` aninhados.
- `tests/erro01.txt` a `tests/erro03.txt`: Casos de erro semântico (atribuição a variável não declarada e uso de variável não declarada dentro de `if` e `while`).
- `tests/test_sintese.sh`: Script de validação automática *end-to-end*.

## 8. Organização da Arquitetura

O projeto adota o Princípio da Responsabilidade Única (SRP) com separação modular por fases do compilador:
- `analise_lexica/`: Implementação do Lexer e mapeamento de Tokens (`lexer.h`/`lexer.c`).
- `analise_sintatica/`: Parser descendente recursivo (`parser.h`/`parser.c`) e estruturas da AST (`ast.h`/`ast.c`).
- `analise_semantica/`: Validador de escopo/comandos (`analise_semantica.c`) e Tabela de Símbolos em Hash Table (`tabela_simbolos.c`).
- `sintese/`: Gerador de código Assembly x86-64 (`gerador.h`/`gerador.c`).
- `runtime.s`: Rotinas de sistema (`imprime_num` e `sair`), incluídas no Assembly gerado.
- `main.c`: Orquestrador do pipeline de compilação e seleção de modo (`--asm` vs padrão).
- `tests/`: Exemplos de entrada válidos, casos de erro e script de testes automatizados.

## 9. Uso de LLMs

Nas atividades anteriores, o grupo utilizou o Google Gemini como apoio na engenharia da Tabela de Símbolos (*djb2*), depuração, testes, documentação e refatoração do `main.c` seguindo princípios de *Clean Code*. Para a **Atividade 08**, o grupo utilizou o **Claude (Anthropic)** como apoio na análise semântica dos comandos, nos arquivos de teste e no esboço da documentação, e o **Google Gemini** no planejamento da arquitetura *Contract-First*, na geração de código x86-64 para comandos/comparações, nas revisões de código e na consolidação deste `README.md`. Todo o código gerado foi revisado, compilado e validado manualmente pelo grupo.