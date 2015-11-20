[BITS 32]

global exit
global write
global writestr
global writebool
global writelf
global read

section .bss

writebuffer: resb 11
writebuffer_end: resb 1

readbuffer: resb 1

section .data

true_str: db `true`
true_len equ $ - true_str

false_str: db `false`
false_len equ $ - false_str

readerror: db `read error: invalid input\n\0`

newline: db `\n`

section .text

exit:
    mov eax, 0x1
    mov ebx, 0x0
    int 0x80

writelf:
    mov eax, 0x4
    mov ebx, 0x1
    lea ecx, [newline]
    mov edx, 0x1
    int 0x80
    ret

write:
    push eax
    mov ecx, 0
    mov ebx, 10
    cmp eax, 0
    jge .loop
    neg eax
.loop:
    mov edx, 0x0
    idiv ebx
    lea edx, [edx+0x30] ; ord('0')
    mov byte [writebuffer_end+ecx], dl
    dec ecx
    test eax, eax
    jnz .loop
    pop eax
    cmp eax, 0
    jge .do
    mov byte [writebuffer_end+ecx], 0x2d ; ord('-')
    dec ecx
.do:
    mov eax, 0x4
    mov ebx, 0x1
    mov edx, ecx
    neg edx
    inc ecx
    lea ecx, [writebuffer_end+ecx]
    int 0x80
    ret

writestr:
    mov edx, eax
    jmp .cond
.loop:
    inc edx
.cond:
    cmp byte [edx], 0
    jnz .loop
    sub edx, eax
    mov ecx, eax
    mov ebx, 0x1
    mov eax, 0x4
    int 0x80
    ret

writebool:
    test eax, eax
    jnz .true
    lea ecx, [false_str]
    mov edx, false_len
    jmp .do
.true:
    lea ecx, [true_str]
    mov edx, true_len
.do:
    mov eax, 0x4
    mov ebx, 0x1
    int 0x80
    ret

read:
    xor eax, eax
    push eax
    push eax
    mov eax, 0x3
    xor ebx, ebx
    lea ecx, [readbuffer]
    mov edx, 0x1
    int 0x80
    test eax, eax
    jz .error
    cmp eax, 1
    jne .error
    mov bl, byte [readbuffer]
    cmp bl, 0x2d ; '-'
    jne .addchr
    mov dword [esp+0x4], 0x1
.loop:
    mov eax, 0x3
    xor ebx, ebx
    lea ecx, [readbuffer]
    mov edx, 0x1
    int 0x80
    test eax, eax
    jz .done
    cmp eax, 1
    jne .error
    mov bl, byte [readbuffer]
    cmp bl, 0xa ;'\n'
    je .done
    cmp bl, 0x20 ;' '
    je .done
    cmp bl, 0x9 ;'\t'
    je .done
.addchr:
    cmp bl, 0x30 ;'0'
    jl .error
    sub bl, 0x30
    cmp bl, 0x9
    jg .error
    mov eax, [esp]
    imul eax, 0xa
    add eax, ebx
    mov [esp], eax
    jmp .loop
.done:
    pop eax
    pop ebx
    test ebx, ebx
    jz .ret
    neg eax
.ret:
    ret
.error:
    lea eax, [readerror]
    call writestr
    call exit
