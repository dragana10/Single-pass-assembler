# file my_test1
.section ivt
.global globGaga


            ldr psw, [psw + 1]

        ldr psw, [psw + 2]
   ldr rsp, [rsp + 3]
   ldr rsp, [rsp + 4]
   ldr rpc, [rpc + 5]

ldr r1, [r1 + 6]
ldr r2, [r2 + 7]
        ldr r3, [r3 + 8]


ldr rpc, $gaga6

        
; ldr r3, $globGaga
; ldr r3, 5
; ldr r3, globGaga
; ldr r3, nedefGaga
; ldr r3, %globGaga
; ldr r3, %nedefGaga
; ldr r3, r1
; ldr r3, [r2]
; ldr r3, [r3 + 5]
; ldr r3, [r4 + globGaga]
; ldr r3, [r6 + nedefGaga]


    .end