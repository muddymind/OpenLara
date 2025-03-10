#include "common_asm.inc"

vertices    .req r0     // arg
count       .req r1     // arg
vx          .req r2
vy          .req r3
vz          .req r4
x           .req vx
y           .req r5
z           .req r6
mx0         .req r7

mx2         .req r8
my2         .req r9
mz2         .req r10
mw2         .req r11
res         .req r12
vg          .req lr

// FIQ regs
my0         .req r8
mz0         .req r9
mw0         .req r10
mx1         .req r11
my1         .req r12
mz1         .req r13
mw1         .req r14

m           .req vx
v           .req vg
mask        .req y

minXY       .req vy
maxXY       .req vz

tmp         .req vy
dz          .req vz
fog         .req vz

SP_MINXY    = 0
SP_MAXXY    = 4
SP_SIZE     = 8

.global transformRoom_asm
transformRoom_asm:
    stmfd sp!, {r4-r11, lr}

    ldr res, =gVerticesBase
    ldr res, [res]
    add res, #VERTEX_G

    ldr tmp, =viewportRel
    ldmia tmp, {minXY, maxXY}
    stmfd sp!, {minXY, maxXY}

    mov mask, #(0xFF << 10)

    ldr m, =gMatrixPtr
    ldr m, [m]
    fiq_on
    ldmia m!, {mx0, my0, mz0, mw0,  mx1, my1, mz1, mw1}
    ldmia m, {mx2, my2, mz2, mw2}^
    fiq_off

.loop:
    // unpack vertex
    ldmia vertices!, {v}

    and vz, mask, v, lsr #6
    and vy, v, #0xFF00
    and vx, mask, v, lsl #10

    // transform z
    mla z, mx2, vx, mw2
    mla z, my2, vy, z
    mla z, mz2, vz, z
    asr z, #FIXED_SHIFT

    // skip if vertex is out of z-range
    add z, #VIEW_OFF
    cmp z, #(VIEW_OFF + VIEW_OFF + VIEW_MAX)
    movhi vg, #(CLIP_NEAR + CLIP_FAR)
    bhi .skip

    and vg, mask, v, lsr #14
    sub z, #VIEW_OFF

    fiq_on
    // transform y
    mla y, mx1, vx, mw1
    mla y, my1, vy, y
    mla y, mz1, vz, y
    asr y, #FIXED_SHIFT

    // transform x
    mla x, mx0, vx, mw0
    mla x, my0, vy, x
    mla x, mz0, vz, x
    asr x, #FIXED_SHIFT
    fiq_off

    // fog
    cmp z, #FOG_MIN
    subgt fog, z, #FOG_MIN
    addgt vg, fog, lsl #6
    lsr vg, #13
    cmp vg, #31
    movgt vg, #31

    // z clipping
    cmp z, #VIEW_MIN
    movle z, #VIEW_MIN
    orrle vg, #CLIP_NEAR
    cmp z, #VIEW_MAX
    movge z, #VIEW_MAX
    orrge vg, #CLIP_FAR

    // project
    mov dz, z, lsr #6
    add dz, z, lsr #4
    divLUT tmp, dz
    mul x, tmp, x
    mul y, tmp, y
    asr x, #(16 - PROJ_SHIFT)
    asr y, #(16 - PROJ_SHIFT)

    // portal rect clipping
    ldmia sp, {minXY, maxXY}

    cmp x, minXY, asr #16
    orrle vg, #CLIP_LEFT
    cmp x, maxXY, asr #16
    orrge vg, #CLIP_RIGHT

    lsl minXY, #16
    lsl maxXY, #16

    cmp y, minXY, asr #16
    orrle vg, #CLIP_TOP
    cmp y, maxXY, asr #16
    orrge vg, #CLIP_BOTTOM

    add x, #(FRAME_WIDTH >> 1)
    add y, #(FRAME_HEIGHT >> 1)

    // frame rect clipping
    cmp x, #FRAME_WIDTH
    cmpls y, #FRAME_HEIGHT
    orrhi vg, #CLIP_FRAME

    // store the result
    strh x, [res, #-6]
    strh y, [res, #-4]
    strh z, [res, #-2]

    mov mask, #(0xFF << 10)
.skip:
    strh vg, [res], #8

    subs count, #1
    bne .loop

    add sp, #SP_SIZE
    ldmfd sp!, {r4-r11, pc}
