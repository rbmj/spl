[BITS 32]

extern exit
extern write
extern writelf
extern writestr
extern writebool
extern read

global _start

section .data

mystr: db `mystring`,0

section .text

_start:
    lea eax, [mystr]
    call writestr
    call writelf
    mov eax, 1
    call writebool
    call writelf
    mov eax, 0
    call writebool
    call writelf
    mov eax, 93849324
    call write
    call writelf
    mov eax, -12823472
    call write
    call writelf
    call read
    call write
    call writelf
    lea eax, [mystr]
    call writestr
    call writelf
    call exit

