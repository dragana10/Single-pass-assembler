# file my_test1
.section ivt
.global globGaga


            ldr r3, $5
ldr r3, $globGaga
ldr r3, 5
ldr r3, globGaga
ldr r3, nedefGaga
ldr r3, %globGaga
ldr r3, %nedefGaga
ldr r3, r1
ldr r3, [r2]
ldr r3, [r3 + 5]
ldr r3, [r4 + globGaga]
ldr r3, [r6 + nedefGaga]


    .end