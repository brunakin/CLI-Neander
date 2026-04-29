# CLI Neander

## ADO 1 - Compiladores

## Etapa 1 — Parser de expressões

### Compilar e rodar (Windows / gcc)

```bash
gcc -std=c99 -Wall -Wextra -o parser_cli parser_cli.c parser.c
.\parser_cli "2+3*(4-1)"
```

Ou sem argumento (ele pede no stdin):

```bash
.\parser_cli
```

### A especificação do tipo de valores (Decimal ou Hexadecimal) é colocada no comando da execução

#### Comandos:

gcc -o neander main.c

./neander binarioNeander.mem -hex

./neander binarioNeander.mem -dec
