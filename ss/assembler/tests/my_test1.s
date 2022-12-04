# file my_test1.s
.section ivt
labelaPRVA: .global gaga
.global gaga
.extern gaga1, gaga2
.extern gaga3,gaga4, gaga5,      gaga6
.section drugaSekcija
.word 0x1234
.word 13
.word 0x5678, 512
.ascii "Hi\n"
.skip 9
.skip 0xF
.equ noviSimb1, 8

    .end