section .text

extern _Z8LoadItemP9HashTablePKDv4_x
extern isalpha

global LoadBuckets            ; predefined entry point name for ldч
;=================================================================================
; LoadBuckets
; Загружает бакеты словами из текста
;
; Input:    rdi = source_data; rsi = end_of_data; rdx = hash_table_ptr
; Output:   none
; Destroys: rax, rcx, rdx, rsi, rdi
;=================================================================================
LoadBuckets:
    push rbp                        ; install the stack frame
    mov  rbp, rsp

    push rbx

    mov  r9,  rdx                   ; hash_table_ptr
    mov  rbx, rdi                   ; source_data
    mov  rdx, rsi                   ; end of source_data


load_new_word:
    cmp  rbx, rdx
    jae  end_of_load_bucket

    ; пропустить не-буквы
    xor  rdi, rdi
    mov  dil, [rbx]
    call isalpha
    test rax, rax
    jnz  start_of_word

    inc  rbx
    jmp  load_new_word

start_of_word:
    ; выделить 32 нулевых байта
    sub  rsp, 32                
    vpxor ymm0, ymm0, ymm0      
    ; mov  rdi, rsp
    vmovdqu [rsp], ymm0


    mov  rsi, rsp                   ; buffer for cur_word
    xor  rdi, rdi
new_letter:
    mov  dil, [rbx]
    call isalpha
    test rax, rax
    jz   end_of_word

    mov  al, [rbx]
    mov  [rsi], al
    inc  rbx
    inc  rsi
    jmp new_letter

end_of_word:

    vmovdqu ymm0, [rsp]               ; (__m256i *)cur_word
    mov  rdi, r9                    ; hash_table_ptr
    mov  rsi, rsp
    push rdx
    push r9
    call _Z8LoadItemP9HashTablePKDv4_x
    pop  r9
    pop  rdx

    add  rsp, 32
    jmp  load_new_word

end_of_load_bucket:
    ; vzeroupper
    pop rbx
    leave
    ret