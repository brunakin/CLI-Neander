#include <stdio.h>
#include <stdint.h>
#include <string.h>

uint8_t AC = 0;
uint8_t PC = 0;
uint8_t IR = 0;      // Registrador de Instrução
uint8_t MAR = 0;     // Memory Address Register
uint8_t MDR = 0;     // Memory Data Register

uint8_t memoria[256];

int flagZ = 0;
int flagN = 0;
int acessos = 0;
int instrucoes = 0;

int modo_step = 0;   // 0 = execução contínua, 1 = passo a passo

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

    fseek(f, 4, SEEK_SET); // pula cabeçalho 03 4E 44 52

    for (int i = 0; i < 256; i++) {
        uint8_t par[2];
        fread(par, 1, 2, f);
        memoria[i] = par[0];
    }

    fclose(f);
}

void imprimir_estado(int usar_hex) {
    if (usar_hex) {
        printf("AC = 0x%02X   PC = 0x%02X   IR = 0x%02X\n", AC, PC, IR);
        printf("MAR = 0x%02X  MDR = 0x%02X\n", MAR, MDR);
    } else {
        printf("AC = %3d   PC = %3d   IR = %3d\n", AC, PC, IR);
        printf("MAR = %3d  MDR = %3d\n", MAR, MDR);
    }
    printf("Flag N = %d   Flag Z = %d\n", flagN, flagZ);
    printf("Acessos a memoria = %d   Instrucoes = %d\n", acessos, instrucoes);
}

void imprimir_memoria(uint8_t mem[256], int usar_hex) {
    for (int i = 0; i < 256; i++) {
        if (mem[i] != 0) {
            if (usar_hex)
                printf("  [0x%02X] = 0x%02X\n", i, mem[i]);
            else
                printf("  [%3d] = %3d\n", i, mem[i]);
        }
    }
}

// Executa uma instrução e retorna 0 se terminou (HLT)
int executar_instrucao() {
    // Fetch
    MAR = PC;
    MDR = memoria[MAR];
    IR = MDR;
    acessos++;

    uint8_t opcode = IR;
    PC++;                     // incrementa PC após buscar instrução

    instrucoes++;

    if (opcode == 0x00) { // NOP
    }
    else if (opcode == 0x10) { // STA
        MAR = memoria[PC];
        MDR = AC;
        memoria[MAR] = MDR;
        acessos += 2;
        PC++;
    }
    else if (opcode == 0x20) { // LDA
        MAR = memoria[PC];
        MDR = memoria[MAR];
        AC = MDR;
        acessos += 2;
        PC++;
        atualiza_flags();
    }
    else if (opcode == 0x30) { // ADD
        MAR = memoria[PC];
        MDR = memoria[MAR];
        AC = AC + MDR;
        acessos += 2;
        PC++;
        atualiza_flags();
    }
    else if (opcode == 0x40) { // OR
        MAR = memoria[PC];
        MDR = memoria[MAR];
        AC = AC | MDR;
        acessos += 2;
        PC++;
        atualiza_flags();
    }
    else if (opcode == 0x50) { // AND
        MAR = memoria[PC];
        MDR = memoria[MAR];
        AC = AC & MDR;
        acessos += 2;
        PC++;
        atualiza_flags();
    }
    else if (opcode == 0x60) { // NOT
        AC = ~AC;
        atualiza_flags();
    }
    else if (opcode == 0x80) { // JMP
        MAR = memoria[PC];
        PC = MAR;
        acessos++;
    }
    else if (opcode == 0x90) { // JN
        MAR = memoria[PC];
        acessos++;
        if (flagN) PC = MAR;
        else PC++;
    }
    else if (opcode == 0xA0) { // JZ
        MAR = memoria[PC];
        acessos++;
        if (flagZ) PC = MAR;
        else PC++;
    }
    else if (opcode == 0xF0) { // HLT
        return 0;
    }
    else {
        // Instrução inválida: apenas avança
        PC++;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo.mem> <-dec|-hex> [-step]\n", argv[0]);
        printf("Ex: %s programa.mem -hex -step\n", argv[0]);
        return 1;
    }

    int usar_hex = 0;
    modo_step = 0;

    // Processa argumentos
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-hex") == 0) usar_hex = 1;
        else if (strcmp(argv[i], "-dec") == 0) usar_hex = 0;
        else if (strcmp(argv[i], "-step") == 0) modo_step = 1;
    }

    carregar_binario(argv[1]);

    uint8_t memoria_antes[256];
    memcpy(memoria_antes, memoria, 256);

    printf("=== MAPA DE MEMORIA ANTES DA EXECUCAO ===\n");
    imprimir_memoria(memoria_antes, usar_hex);

    printf("\n=== INICIANDO EXECUCAO ===\n");

    if (modo_step) {
        printf("Modo passo a passo ativado. Pressione ENTER para executar cada instrução...\n");
        while (executar_instrucao()) {
            imprimir_estado(usar_hex);
            printf("----------------------------------------\n");
            getchar();  // aguarda pressionar ENTER
        }
    } else {
        while (executar_instrucao()) {
            // loop vazio - executa até HLT
        }
    }

    printf("\n=== RESULTADO FINAL ===\n");
    imprimir_estado(usar_hex);

    printf("\n=== MAPA DE MEMORIA APOS EXECUCAO ===\n");
    imprimir_memoria(memoria, usar_hex);

    return 0;
}