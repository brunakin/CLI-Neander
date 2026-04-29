#include "assembler.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIMBOLOS 120
#define TAM_SIMBOLO  32
#define TAM_LINHA    256

typedef struct {
    char nome[TAM_SIMBOLO];
    unsigned char endereco;
} Simbolo;

static Simbolo tabela[MAX_SIMBOLOS];
static int total_simbolos = 0;

static int tem_erro = 0;
static int linha_erro = 0;
static char texto_erro[256];

// Guarda só o primeiro erro encontrado
static void registrar_erro(int linha, const char* msg) {
    if (tem_erro) return;
    tem_erro = 1;
    linha_erro = linha;
    snprintf(texto_erro, sizeof(texto_erro), "%s", msg);
}

// Coloca a string em maiúsculo 
static void para_maiusculo(char* s) {
    if (!s) return;
    for (int i = 0; s[i]; i++) s[i] = (char)toupper((unsigned char)s[i]);
}

// Remove comentários e remove \r\n do final
static void limpar_linha(char* linha) {
    if (!linha) return;

    char* p = strchr(linha, ';');
    if (p) *p = '\0';
    p = strchr(linha, '#');
    if (p) *p = '\0';

    size_t n = strlen(linha);
    while (n > 0 && (linha[n - 1] == '\r' || linha[n - 1] == '\n')) {
        linha[n - 1] = '\0';
        n--;
    }
}

// Busca símbolo na tabela 
static int buscar_simbolo(const char* nome) {
    for (int i = 0; i < total_simbolos; i++) {
        if (strcmp(tabela[i].nome, nome) == 0) return (int)tabela[i].endereco;
    }
    return -1;
}

// Adiciona símbolo novo
static void adicionar_simbolo(int linha, const char* nome, int endereco) {
    if (total_simbolos >= MAX_SIMBOLOS) {
        registrar_erro(linha, "Tabela de símbolos cheia");
        return;
    }
    if (buscar_simbolo(nome) != -1) {
        registrar_erro(linha, "Rótulo duplicado");
        return;
    }
    strncpy(tabela[total_simbolos].nome, nome, TAM_SIMBOLO - 1);
    tabela[total_simbolos].nome[TAM_SIMBOLO - 1] = '\0';
    tabela[total_simbolos].endereco = (unsigned char)endereco;
    total_simbolos++;
}

// Retorna opcode e se usa operando (ISA do seu executor) 
static unsigned char obter_opcode(const char* inst, int* usa_operando) {
    *usa_operando = 0;

    if (strcmp(inst, "NOP") == 0) return 0x00;
    if (strcmp(inst, "STA") == 0) { *usa_operando = 1; return 0x10; }
    if (strcmp(inst, "LDA") == 0) { *usa_operando = 1; return 0x20; }
    if (strcmp(inst, "ADD") == 0) { *usa_operando = 1; return 0x30; }
    if (strcmp(inst, "OR")  == 0) { *usa_operando = 1; return 0x40; }
    if (strcmp(inst, "AND") == 0) { *usa_operando = 1; return 0x50; }
    if (strcmp(inst, "NOT") == 0) return 0x60;
    if (strcmp(inst, "JMP") == 0) { *usa_operando = 1; return 0x80; }
    if (strcmp(inst, "JN")  == 0) { *usa_operando = 1; return 0x90; }
    if (strcmp(inst, "JZ")  == 0) { *usa_operando = 1; return 0xA0; }
    if (strcmp(inst, "HLT") == 0) return 0xF0;

    return 0xFF;
}

// Lê número decimal ou 0x
static int eh_numero(const char* s, long* valor) {
    if (!s) return 0;
    char* fim = NULL;
    *valor = strtol(s, &fim, 0);
    return (fim && *fim == '\0');
}

static int resolver_operando(int linha, const char* op, int* end) {
    if (!op) {
        registrar_erro(linha, "Faltou operando na instrução");
        return 0;
    }

    long v = 0;
    if (eh_numero(op, &v)) {
        if (v < 0 || v > 255) {
            registrar_erro(linha, "Operando fora do intervalo (0..255)");
            return 0;
        }
        *end = (int)v;
        return 1;
    }

    char nome[TAM_SIMBOLO];
    strncpy(nome, op, sizeof(nome) - 1);
    nome[sizeof(nome) - 1] = '\0';
    para_maiusculo(nome);

    int achou = buscar_simbolo(nome);
    if (achou == -1) {
        registrar_erro(linha, "Rótulo não encontrado");
        return 0;
    }

    *end = achou;
    return 1;
}

// Primeira Passagem
static void primeira_passagem(FILE* f) {
    char linha[TAM_LINHA];
    int pc = 0;
    int num_linha = 0;

    total_simbolos = 0;
    tem_erro = 0;
    linha_erro = 0;
    texto_erro[0] = '\0';

    while (fgets(linha, sizeof(linha), f) && !tem_erro) {
        num_linha++;
        limpar_linha(linha);
        if (linha[0] == '\0') continue;

        char* t1 = strtok(linha, " \t");
        if (!t1) continue;

        char* t2 = strtok(NULL, " \t");
        char* t3 = strtok(NULL, " \t");

        // Tratamento de rótulo com ':'
        if (t1[strlen(t1) - 1] == ':') {
            t1[strlen(t1) - 1] = '\0';
            para_maiusculo(t1);
            adicionar_simbolo(num_linha, t1, pc);
            t1 = t2;
            t2 = t3;
            t3 = strtok(NULL, " \t");
            if (!t1) continue;
        }

        para_maiusculo(t1);
        if (t2) para_maiusculo(t2);

        // Tratamento de rótulo sem ':' seguido de DATA ou SPACE 
        if (t2 && (strcmp(t2, "DATA") == 0 || strcmp(t2, "SPACE") == 0)) {
            adicionar_simbolo(num_linha, t1, pc); 
            t1 = t2;                            
            t2 = t3;
            t3 = strtok(NULL, " \t");
            if (t2) para_maiusculo(t2);
        }

        if (strcmp(t1, "ORG") == 0) {
            long v = 0;
            if (!t2 || !eh_numero(t2, &v) || v < 0 || v > 255) {
                registrar_erro(num_linha, "ORG inválido");
                continue;
            }
            pc = (int)v;
            continue;
        }

        if (strcmp(t1, "DATA") == 0) {
            pc += 1;
            continue;
        }

        if (strcmp(t1, "SPACE") == 0) {
            long tam = 0;
            if (!t2 || !eh_numero(t2, &tam) || tam < 0) {
                registrar_erro(num_linha, "SPACE inválido");
                continue;
            }
            pc += (int)tam;
            continue;
        }

        // Instrução comum
        {
            int usa_op = 0;
            unsigned char op = obter_opcode(t1, &usa_op);
            if (op == 0xFF) {
                registrar_erro(num_linha, "Instrução inválida");
                continue;
            }

            if (usa_op) {
                if (!t2) registrar_erro(num_linha, "Falta operando");
                pc += 2;
            } else {
                if (t2) registrar_erro(num_linha, "Instrução não deve ter operando");
                pc += 1;
            }
        }

        if (pc > 256) registrar_erro(num_linha, "Programa estourou 256 bytes");
    }
}

// Segunda Passagem
static void segunda_passagem(FILE* f, unsigned char mem[256]) {
    char linha[TAM_LINHA];
    int pc = 0;
    int num_linha = 0;

    memset(mem, 0, 256);
    rewind(f);

    while (fgets(linha, sizeof(linha), f) && !tem_erro) {
        num_linha++;
        limpar_linha(linha);
        if (linha[0] == '\0') continue;

        char* t1 = strtok(linha, " \t");
        if (!t1) continue;

        char* t2 = strtok(NULL, " \t");
        char* t3 = strtok(NULL, " \t");

        if (t1[strlen(t1) - 1] == ':') {
            t1 = t2;
            t2 = t3;
            t3 = strtok(NULL, " \t");
            if (!t1) continue;
        }

        para_maiusculo(t1);
        if (t2) para_maiusculo(t2);

        if (t2 && (strcmp(t2, "DATA") == 0 || strcmp(t2, "SPACE") == 0)) {
            t1 = t2;
            t2 = t3;
            t3 = strtok(NULL, " \t");
            if (t2) para_maiusculo(t2);
        }

        if (strcmp(t1, "ORG") == 0) {
            long v = 0;
            if (t2 && eh_numero(t2, &v)) pc = (int)v;
            continue;
        }

        if (strcmp(t1, "DATA") == 0) {
            long v = 0;
            if (t2 && eh_numero(t2, &v) && v >= 0 && v <= 255) mem[pc] = (unsigned char)v;
            pc++;
            continue;
        }

        if (strcmp(t1, "SPACE") == 0) {
            long tam = 0;
            if (t2 && eh_numero(t2, &tam) && tam >= 0) pc += (int)tam;
            if (pc > 256) registrar_erro(num_linha, "SPACE estourou 256 bytes");
            continue;
        }

        {
            int usa_op = 0;
            unsigned char op = obter_opcode(t1, &usa_op);
            if (op == 0xFF) continue;

            mem[pc++] = op;

            if (usa_op) {
                int end = 0;
                if (t2 && resolver_operando(num_linha, t2, &end)) mem[pc++] = (unsigned char)end;
            }
        }

        if (pc > 256) registrar_erro(num_linha, "Programa estourou 256 bytes");
    }
}

// Salvar Arquivo
static int salvar_mem(const char* nome, const unsigned char mem[256]) {
    FILE* out = fopen(nome, "wb");
    if (!out) return 0;

    unsigned char cab[4] = {0x03, 0x4E, 0x44, 0x52};
    fwrite(cab, 1, 4, out);

    for (int i = 0; i < 256; i++) {
        unsigned char par[2] = {mem[i], 0x00};
        fwrite(par, 1, 2, out);
    }

    fclose(out);
    return 1;
}

int montar_arquivo(const char* caminho_asm,
                   const char* caminho_mem,
                   char* msg_erro,
                   size_t tam_msg_erro) {
 
    if (!msg_erro || tam_msg_erro == 0) return 0;
    msg_erro[0] = '\0';

    FILE* f = fopen(caminho_asm, "r");
    if (!f) {
        snprintf(msg_erro, tam_msg_erro, "Não foi possível abrir o arquivo .asm");
        return 0;
    }

    primeira_passagem(f);
    if (tem_erro) {
        snprintf(msg_erro, tam_msg_erro, "Linha %d: %s", linha_erro, texto_erro);
        fclose(f);
        return 0;
    }

    unsigned char mem[256];
    segunda_passagem(f, mem);
    fclose(f);

    if (tem_erro) {
        snprintf(msg_erro, tam_msg_erro, "Linha %d: %s", linha_erro, texto_erro);
        return 0;
    }

    if (!salvar_mem(caminho_mem, mem)) {
        snprintf(msg_erro, tam_msg_erro, "Erro ao gravar o arquivo .mem");
        return 0;
    }

    return 1;
}