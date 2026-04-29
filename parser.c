#include "parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef enum TipoToken { // cada "pedaço" vira um token 
    TOK_FIM = 0,         // acabou a expressão 
    TOK_NUM,             // número 
    TOK_MAIS,            // + 
    TOK_MENOS,           // - 
    TOK_VEZES,           // * 
    TOK_DIV,             // / 
    TOK_ABRE,            // ( 
    TOK_FECHA,           // ) 
    TOK_INVALIDO         // caractere que não faz parte da linguagem 
} TipoToken;

typedef struct Token {
    TipoToken tipo; // tipo do token 
    size_t pos;     // posição na string (pra indicar erro) 
    long numero;    // valor do número (só quando tipo == TOK_NUM) 
} Token;

#define MAX_TOKENS 512

static Token tokens[MAX_TOKENS]; // tokens da entrada 
static int qtd_tokens = 0;       // quantos tokens já foram gerados 
static int idx = 0;              // índice do token atual 
static int deu_erro = 0;         // se der erro, fica 1 
static char texto_erro[128];     // mensagem do erro 

static void erro(const char* msg) {
    if (deu_erro) return; // não troca a mensagem se já errou 
    deu_erro = 1;         // marca erro 
    snprintf(texto_erro, sizeof(texto_erro), "%s", msg); // salva a mensagem 
}

static Token token_atual(void) {
    if (idx < 0) idx = 0; // segurança 
    if (idx >= qtd_tokens) return tokens[qtd_tokens - 1]; // retorna o último (geralmente FIM) 
    return tokens[idx]; // token na posição atual 
}

static void avancar(void) {
    if (idx < qtd_tokens) idx++;
}

static void adicionar_token(TipoToken tipo, size_t pos, long numero) {
    if (qtd_tokens >= MAX_TOKENS) return; // se estourar, ignora (bem simples) 
    tokens[qtd_tokens].tipo = tipo;       // guarda tipo 
    tokens[qtd_tokens].pos = pos;         // guarda posição 
    tokens[qtd_tokens].numero = numero;   // guarda valor numérico 
    qtd_tokens++;                         // aumenta contador 
}

static void tokenizar(const char* entrada) {
    qtd_tokens = 0; // reseta a lista 
    idx = 0;        // começa do primeiro 

    size_t i = 0;
    while (entrada[i]) {
        unsigned char c = (unsigned char)entrada[i];
        if (isspace(c)) {
            i++;
            continue;
        }

        if (isdigit(c)) {
            long v = 0;
            size_t pos_ini = i;
            while (isdigit((unsigned char)entrada[i])) {
                v = v * 10 + (entrada[i] - '0'); // base 10 
                i++;                              // anda na string 
            }
            adicionar_token(TOK_NUM, pos_ini, v);
            continue;
        }

        switch (entrada[i]) {
            case '+': adicionar_token(TOK_MAIS, i, 0); i++; break;
            case '-': adicionar_token(TOK_MENOS, i, 0); i++; break;
            case '*': adicionar_token(TOK_VEZES, i, 0); i++; break;
            case '/': adicionar_token(TOK_DIV, i, 0); i++; break;
            case '(': adicionar_token(TOK_ABRE, i, 0); i++; break;
            case ')': adicionar_token(TOK_FECHA, i, 0); i++; break;
            default:
                adicionar_token(TOK_INVALIDO, i, 0);
                i++;
                break;
        }
    }
    adicionar_token(TOK_FIM, i, 0);
}

static long expressao(void); // declaração adiantada (porque fator chama expressao) 

static long fator(void) {
    Token t = token_atual();

    if (t.tipo == TOK_INVALIDO) {
        erro("Caractere inválido na expressão");
        return 0;
    }

    if (t.tipo == TOK_NUM) {
        avancar();     
        return t.numero; 
    }

    if (t.tipo == TOK_ABRE) {
        avancar();
        long v = expressao();
        if (!deu_erro) {
            if (token_atual().tipo != TOK_FECHA) erro("Era esperado ')'");
            else avancar();
        }
        return v;
    }

    if (t.tipo == TOK_MAIS || t.tipo == TOK_MENOS) {
        avancar();        // consome o sinal 
        long v = fator(); // aplica no próximo fator
        if (t.tipo == TOK_MENOS) v = -v; // se for -, inverte 
        return v;      
    }

    erro("Era esperado número, '(' ou operador unário");
    return 0;
}

static long termo(void) {
    long v = fator();
    while (!deu_erro) {
        Token t = token_atual();
        if (t.tipo != TOK_VEZES && t.tipo != TOK_DIV) break;
        avancar();

        long dir = fator();
        if (deu_erro) return 0;

        if (t.tipo == TOK_VEZES) {
            v = v * dir;
        } else {
            if (dir == 0) {
                erro("Divisão por zero");
                return 0;
            }
            v = v / dir;
        }
    }
    return v;
}

static long expressao(void) {
    long v = termo();
    while (!deu_erro) {
        Token t = token_atual();
        if (t.tipo != TOK_MAIS && t.tipo != TOK_MENOS) break;
        avancar();

        long dir = termo();
        if (deu_erro) return 0;
        if (t.tipo == TOK_MAIS) v = v + dir;
        else v = v - dir;
    }
    return v;
}

int analisar_expressao(const char* entrada,
                       long* resultado,
                       char* msg_erro,
                       size_t tam_msg_erro) {
    if (!entrada || !resultado || !msg_erro || tam_msg_erro == 0) return 0;

    deu_erro = 0;         // reseta erro 
    texto_erro[0] = '\0'; // limpa msg interna 
    msg_erro[0] = '\0';   // limpa msg de saída 

    tokenizar(entrada);
    if (token_atual().tipo == TOK_INVALIDO) {
        erro("Caractere inválido na expressão");
    }

    long v = 0;
    if (!deu_erro) v = expressao();

    if (!deu_erro && token_atual().tipo != TOK_FIM) {
        erro("Sobrou texto após o fim da expressão");
    }

    if (deu_erro) {
        snprintf(msg_erro, tam_msg_erro, "%s (posição %zu)", texto_erro, token_atual().pos);
        return 0;
    }

    *resultado = v;
    return 1;
}

// Gerador de Assembly para expressões como "2+3*4" e "10-3"
int gerar_asm_da_expressao(const char* expressao_original, long resultado, const char* nome_arquivo_asm) {
    FILE* f = fopen(nome_arquivo_asm, "w");
    if (!f) {
        printf("Erro ao criar arquivo .asm: %s\n", nome_arquivo_asm);
        return 0;
    }

    fprintf(f, "; ==================================================\n");
    fprintf(f, "; Expressão: %s\n", expressao_original);
    fprintf(f, "; Resultado calculado: %ld\n", resultado);
    fprintf(f, "; ==================================================\n\n");

    fprintf(f, "ORG 0\n\n");

    // Se a expressão for simples (sem comparação), gera código linear
    if (strstr(expressao_original, ">") == NULL && 
        strstr(expressao_original, "<") == NULL && 
        strstr(expressao_original, "==") == NULL) {
        
        fprintf(f, "; Código simples - resultado direto\n");
        fprintf(f, "LDA VALOR_RESULTADO\n");
        fprintf(f, "STA SAIDA\n");
        fprintf(f, "HLT\n\n");
    } 
    else {
        // Exemplo básico com condição (por enquanto só suporta expressões como "A > B")
        fprintf(f, "; Exemplo com salto condicional\n");
        fprintf(f, "LDA A\n");
        fprintf(f, "SUB B\n");           // A - B
        fprintf(f, "JN  EH_NEGATIVO\n"); // Se resultado negativo → pula
        fprintf(f, "LDA UM\n");          // Caso positivo ou zero
        fprintf(f, "STA RESULTADO\n");
        fprintf(f, "JMP  FIM\n");
        
        fprintf(f, "EH_NEGATIVO:\n");
        fprintf(f, "LDA ZERO\n");
        fprintf(f, "STA RESULTADO\n");
        
        fprintf(f, "FIM:\n");
        fprintf(f, "HLT\n\n");
    }

    // Dados
    fprintf(f, "; --- Dados ---\n");
    fprintf(f, "ORG 100\n");
    fprintf(f, "A:      DATA 10\n");
    fprintf(f, "B:      DATA 3\n");
    fprintf(f, "UM:     DATA 1\n");
    fprintf(f, "ZERO:   DATA 0\n");
    fprintf(f, "VALOR_RESULTADO: DATA %ld\n", resultado);
    fprintf(f, "SAIDA:           SPACE 1\n");
    fprintf(f, "RESULTADO:       SPACE 1\n");

    fclose(f);
    printf("✓ Arquivo .asm gerado com suporte a saltos: %s\n", nome_arquivo_asm);
    return 1;
}

