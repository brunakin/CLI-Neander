//A especificação do tipo de valores (Decimal ou Hexadecimal) é colocada no comando da execução

//Comandos:
//gcc -o neander main.c
//./neander binarioNeander.mem -hex
//./neander binarioNeander.mem -dec


#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint8_t AC = 0;
uint8_t PC = 0;
uint8_t memoria[256];
int flagZ = 0;
int flagN = 0;
int acessos = 0;
int instrucoes = 0;

void atualiza_flags() {
    flagZ = (AC == 0) ? 1 : 0;
    flagN = (AC >> 7) ? 1 : 0;
}

void carregar_binario(const char* caminho) {
    FILE* f = fopen(caminho, "rb");
    if (!f) {
        printf("Erro ao abrir arquivo!\n");
        return;
    }

    fseek(f, 4, SEEK_SET); // pula cabeçalho de 4 bytes

    for (int i = 0; i < 256; i++) {
        uint8_t par[2];
        fread(par, 1, 2, f);
        memoria[i] = par[0];
    }

    fclose(f);
}

void executar() {
    int rodando = 1;
    while (rodando) {
        uint8_t opcode = memoria[PC];
        acessos++;
        instrucoes++;

        if (opcode == 0x00) { // NOP
            PC += 1;

        } else if (opcode == 0x20) { // LDA
            uint8_t end = memoria[PC + 1];
            acessos += 2;
            AC = memoria[end];
            PC += 2;
            atualiza_flags();

        } else if (opcode == 0x30) { // ADD
            uint8_t end = memoria[PC + 1];
            acessos += 2;
            AC = AC + memoria[end];
            PC += 2;
            atualiza_flags();

        } else if (opcode == 0x40) { // OR
            uint8_t end = memoria[PC + 1];
            acessos += 2;
            AC = AC | memoria[end];
            PC += 2;
            atualiza_flags();

        } else if (opcode == 0x50) { // AND
            uint8_t end = memoria[PC + 1];
            acessos += 2;
            AC = AC & memoria[end];
            PC += 2;
            atualiza_flags();

        } else if (opcode == 0x60) { // NOT
            AC = ~AC;
            PC += 1;
            atualiza_flags();

        } else if (opcode == 0x10) { // STA
            uint8_t end = memoria[PC + 1];
            acessos += 2;
            memoria[end] = AC;
            PC += 2;

        } else if (opcode == 0x80) { // JMP
            uint8_t end = memoria[PC + 1];
            acessos++;
            PC = end;

        } else if (opcode == 0x90) { // JN
            uint8_t end = memoria[PC + 1];
            acessos++;
            if (flagN) PC = end;
            else PC += 2;

        } else if (opcode == 0xA0) { // JZ
            uint8_t end = memoria[PC + 1];
            acessos++;
            if (flagZ) PC = end;
            else PC += 2;

        } else if (opcode == 0xF0) { // HLT
            rodando = 0;

        } else {
            PC += 1;
        }
    }
}

void imprimir_estado(int usar_hex) {
    if (usar_hex) {
        printf("AC = 0x%02X\n", AC);
        printf("PC = 0x%02X\n", PC);
    } else {
        printf("AC = %d\n", AC);
        printf("PC = %d\n", PC);
    }
    printf("Flag N = %d\n", flagN);
    printf("Flag Z = %d\n", flagZ);
}

void imprimir_memoria(uint8_t mem[256], int usar_hex) {
    for (int i = 0; i < 256; i++) {
        if (mem[i] != 0) {
            if (usar_hex) printf("  [0x%02X] = 0x%02X\n", i, mem[i]);
            else          printf("  [%3d] = %d\n", i, mem[i]);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo.mem> <-dec|-hex>\n", argv[0]);
        return 1;
    }

    int usar_hex = 0;
    if (strcmp(argv[2], "-hex") == 0) usar_hex = 1;
    else if (strcmp(argv[2], "-dec") == 0) usar_hex = 0;
    else {
        printf("Opcao invalida! Use -dec ou -hex.\n");
        return 1;
    }

    carregar_binario(argv[1]);

    uint8_t memoria_antes[256];
    memcpy(memoria_antes, memoria, 256);

    printf("=== MAPA DE MEMORIA ANTES DA EXECUCAO ===\n");
    imprimir_memoria(memoria_antes, usar_hex);

    executar();

    printf("\n=== RESULTADO APOS EXECUCAO ===\n");
    imprimir_estado(usar_hex);
    printf("Acessos a memoria  = %d\n", acessos);
    printf("Instrucoes executadas = %d\n", instrucoes);

    printf("\n=== MAPA DE MEMORIA DEPOIS DA EXECUCAO ===\n");
    imprimir_memoria(memoria, usar_hex);

    return 0;
}