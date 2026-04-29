// principal.c - FLUXO COMPLETO: Parser → Assembler → Executor
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>     

// parser.c
int analisar_expressao(const char* entrada, long* resultado, char* msg_erro, size_t tam_msg_erro);
int gerar_asm_da_expressao(const char* expressao_original, long resultado, const char* nome_arquivo_asm);

// assembler.c
int montar_arquivo(const char* caminho_asm, const char* caminho_mem, 
                   char* msg_erro, size_t tam_msg_erro);

// executor (main.c)
void inicializar_executor(void);
void executar_programa(const char* arquivo_mem, int usar_hex, int step);

void carregar_binario(const char* caminho);
int executar_instrucao(void);
void imprimir_estado(int usar_hex);
void imprimir_memoria(uint8_t mem[256], int usar_hex);

extern uint8_t AC, PC, IR, MAR, MDR;
extern uint8_t memoria[256];
extern int flagZ, flagN, acessos, instrucoes, modo_step;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s \"expressao\" [-dec|-hex] [-step]\n", argv[0]);
        printf("Exemplo: %s \"2+3*(4-1)\" -hex\n", argv[0]);
        return 1;
    }

    const char* expressao = argv[1];
    int usar_hex = 0;
    int usar_step = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-hex") == 0) usar_hex = 1;
        else if (strcmp(argv[i], "-dec") == 0) usar_hex = 0;
        else if (strcmp(argv[i], "-step") == 0) usar_step = 1;
    }

    printf("=== NEANDER - FLUXO COMPLETO ===\n\n");

    // 1. PARSER
    long resultado = 0;
    char erro[256] = {0};

    printf("1. PARSER\n");
    if (!analisar_expressao(expressao, &resultado, erro, sizeof(erro))) {
        printf("Erro no parser: %s\n", erro);
        return 1;
    }
    printf("Expressao : %s\n", expressao);
    printf("Resultado : %ld\n\n", resultado);

    // 2. GERAR .asm
    printf("2. GERANDO .asm\n");
    if (!gerar_asm_da_expressao(expressao, resultado, "programa_gerado.asm")) {
        printf("Falha ao gerar o arquivo .asm\n");
        return 1;
    }

    // 3. ASSEMBLER
    printf("\n3. ASSEMBLER\n");
    char erro_asm[256] = {0};
    if (!montar_arquivo("programa_gerado.asm", "programa_gerado.mem", erro_asm, sizeof(erro_asm))) {
        printf("Erro no assembler: %s\n", erro_asm);
        return 1;
    }
    printf(".mem gerado com sucesso!\n\n");

    // 4. EXECUTOR
    printf("4. EXECUTOR\n");
    inicializar_executor();
    executar_programa("programa_gerado.mem", usar_hex, usar_step);

    printf("\n=== FIM DO FLUXO COMPLETO ===\n");
    return 0;
}