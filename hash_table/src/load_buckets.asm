section .text

global LoadBuckets            ; predefined entry point name for ldч
extern LoadItem

;=================================================================================
; LoadBuckets
; Загружает бакеты словами из текста
;
; Input:    rdi = end_of_data; rsi = source_data; rdx = hash_table_ptr
; Output:   none
; Destroys: rax, rcx, rdx, rsi, rdi
;=================================================================================
LoadBuckets:
    push rbp                        ; install the stack frame
    mov  rbp, rsp

    mov  r9,  rdx                   ; hash_table_ptr
    mov  rbx, rdi                   ; source_data
    mov  rdx, rsi                   ; end of source_data


load_new_word:
    cmp  rbx, rdx
    jae  end_of_load_bucket

    ; пропустить не-буквы
    mov  rdi, byte ptr [rbx]
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
new_letter:
    mov  rdi, byte ptr [rbx]
    call isalpha
    test rax, rax
    jz   end_of_word

    mov  al, [rbx]
    mov  [rsi], al
    inc  rbx
    inc  rsi
    jmp new_letter

end_of_word:

    vmovdqu ymm0, rsp               ; (__m256i *)cur_word
    mov  rdi, r9                    ; hash_table_ptr
    call LoadItem

    jmp  load_new_word

end_of_load_bucket:
    leave
    ret