#   testdir1.s

.section ivtp
  .word main
  .skip 2
  .word timer
  .word keyboard
  .skip 8

.extern print
.extern A_location
.extern B_value
.extern B_text

.global PRINT_REG

.equ PRINT_REG, 0xFF00

.section text
  keyboard:
    push r0
    ldr r0, $asciiCode
    str r0, term_out
    pop r0
    iret
  timer:
    iret
  main:
int r0
int r7
int rsp
and r1, r4
cmp  r2,  r3
	

.section data
  A_text: .word 65, 0x20, 86, 65, 76, 85, 69, 0x3A, 0x20, 0, 0xFFEE
.end