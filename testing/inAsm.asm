bits 64
section .text
; nasm testing/inAsm.asm -felf64 && clang testing/inAsm.o -Wall -Wextra -g testing/test.c

;  read_input arr, length, counter, nextCharReg, done_location
%macro read_input 5
  cmp %3, %2 ; counter >= length, goto done
  jge %5
  mov %4, byte [%1 + %3] ; load next
  cmp %4, 0 ; if null char, goto end
  je %5
  add %3, 1 ; inc count
%endmacro


global match_aaStar
; args: rdi=arr, rsi=length
; locals: edx=counter, eax=longestMatch, cl=nextCharReg
match_aaStar:
  xor edx, edx ; counter
  mov rax, -1 ; longestMatch

  .state_1:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'a'
  je .state_2
  jmp .state_done

  .state_2:
  mov rax, rdx
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'a'
  je .state_2
  jmp .state_done


  .state_done:
  ret

global match_borcd
; args: rdi=arr, rsi=length
; locals: edx=counter, eax=longestMatch, cl=nextCharReg
match_borcd:
  xor edx, edx ; counter
  mov rax, -1 ; longestMatch

  .state_1:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'b'
  je .state_2
  cmp cl, 'c'
  je .state_2
  jmp .state_done

  .state_2:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'd'
  je .state_3
  jmp .state_done

  .state_3:
  mov rax, rdx
  jmp .state_done

  .state_done:
  ret


global match_bacStard
; args: rdi=arr, rsi=length
; locals: edx=counter, eax=longestMatch, cl=nextCharReg
match_bacStard:
  xor edx, edx ; counter
  mov rax, -1 ; longestMatch

  .state_1:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'b'
  je .state_2
  cmp cl, 'd'
  je .state_5
  cmp cl, 'c'
  je .state_4
  jmp .state_done

  .state_2:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'a'
  je .state_3
  jmp .state_done

  .state_3:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'd'
  je .state_5
  jmp .state_done

  .state_4:
  read_input rdi, rsi, rdx, cl, .state_done
  cmp cl, 'd'
  je .state_5
  cmp cl, 'c'
  je .state_4
  jmp .state_done

  .state_5:
  mov rax, rdx
  jmp .state_done

  .state_done:
  ret
