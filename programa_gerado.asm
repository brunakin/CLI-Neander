; ==================================================
; Expressão original: 2+3*4
; Resultado calculado pelo parser: 14
; ==================================================

ORG 0
LDA VALOR_RESULTADO
STA SAIDA
HLT

; --- Dados ---
ORG 100
VALOR_RESULTADO: DATA 14
SAIDA:           SPACE 1
