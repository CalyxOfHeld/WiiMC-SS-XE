/* wiifin_tramp.S: puts a DOL (staged in MEM2) in place and starts it, its
 * "_arg" block handed over.  WiiFin's own (source/core/Reload.cpp there).
 * r3: the DOL; r4: the six words of its "_arg" block, or 0.  Registers
 * only: it runs once everything else has stopped. */
    .text
    .balign 32
    .globl wiifinTrampoline
    .globl wiifinTrampolineEnd
wiifinTrampoline:
    mfmsr   5
    rlwinm  5, 5, 0, 17, 15         /* interrupts off */
    mtmsr   5
    li      6, 0                    /* the 18 sections (7 text, 11 data) */
1:  lwzx    7, 3, 6                 /* its offset in the file */
    addi    8, 6, 0x48
    lwzx    8, 3, 8                 /* its address */
    addi    9, 6, 0x90
    lwzx    9, 3, 9                 /* its size */
    cmpwi   9, 0
    beq     4f
    add     7, 7, 3
    mr      10, 8
    mr      11, 9
2:  lwz     12, 0(7)
    stw     12, 0(10)
    addi    7, 7, 4
    addi    10, 10, 4
    addic.  11, 11, -4
    bgt     2b
    mr      10, 8
    add     11, 8, 9
3:  dcbf    0, 10                   /* to memory, out of the instruction cache */
    icbi    0, 10
    addi    10, 10, 32
    cmplw   10, 11
    blt     3b
4:  addi    6, 6, 4
    cmpwi   6, 72
    blt     1b
    lwz     8, 0xD8(3)              /* the BSS, cleared */
    lwz     9, 0xDC(3)
    cmpwi   9, 0
    beq     6f
    li      12, 0
    mr      10, 8
    mr      11, 9
5:  stw     12, 0(10)
    addi    10, 10, 4
    addic.  11, 11, -4
    bgt     5b
    mr      10, 8
    add     11, 8, 9
55: dcbf    0, 10
    addi    10, 10, 32
    cmplw   10, 11
    blt     55b
6:  sync
    isync
    lwz     8, 0xE0(3)              /* its entry */
    cmpwi   4, 0
    beq     8f
    lis     10, 0x5f61              /* "_arg" after its first branch (or its */
    ori     10, 10, 0x7267          /* second: WiiFin's crt0_pre), its block after */
    addi    5, 8, 4
    lwz     9, 0(5)
    cmpw    9, 10
    beq     65f
    addi    5, 8, 8
    lwz     9, 0(5)
    cmpw    9, 10
    bne     8f
65: addi    5, 5, 4
    li      6, 0
7:  lwzx    9, 4, 6
    stwx    9, 5, 6
    addi    6, 6, 4
    cmpwi   6, 24
    blt     7b
    dcbf    0, 5
    addi    10, 5, 24
    dcbf    0, 10
    sync
    icbi    0, 5
    isync
8:  mtctr   8
    bctr
wiifinTrampolineEnd:
    nop
