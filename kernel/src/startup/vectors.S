.syntax unified
.cpu cortex-a9
.arm

.section .vectors, "ax", %progbits
.global minemu_vectors
.type minemu_vectors, %function
.extern minemu_fail_stop
.extern minemu_svc_trampoline
.extern minemu_irq_trampoline
.extern minemu_undefined_dispatch
.extern minemu_abort_dispatch

minemu_vectors:
    b minemu_fail_stop
    b minemu_undefined_entry
    b minemu_svc_trampoline
    b minemu_prefetch_abort_entry
    b minemu_data_abort_entry
    .word 0
    b minemu_irq_trampoline
    .word 0

minemu_undefined_entry:
    sub sp, sp, #88
    stmia sp, {r0-r12}
    str lr, [sp, #52]
    mrs r1, spsr
    str r1, [sp, #56]
    ldr r1, =-4
    str r1, [sp, #60]
    sub r1, lr, #4
    str r1, [sp, #64]
    mov r1, #0
    str r1, [sp, #68]
    str r1, [sp, #72]
    add r1, sp, #76
    stmia r1, {sp, lr}^
    mov r1, #0
    str r1, [sp, #84]
    mov r0, sp
    bl minemu_undefined_dispatch
    ldr lr, [sp, #52]
    ldr r1, [sp, #56]
    msr spsr_cxsf, r1
    add r1, sp, #76
    ldmia r1, {sp, lr}^
    ldmia sp, {r0-r12}
    add sp, sp, #88
    movs pc, lr

minemu_prefetch_abort_entry:
    sub sp, sp, #88
    stmia sp, {r0-r12}
    str lr, [sp, #52]
    mrs r1, spsr
    str r1, [sp, #56]
    ldr r1, =-2
    str r1, [sp, #60]
    sub r1, lr, #4
    str r1, [sp, #64]
    mrc p15, 0, r1, c5, c0, 0
    str r1, [sp, #68]
    mrc p15, 0, r1, c6, c0, 0
    str r1, [sp, #72]
    add r1, sp, #76
    stmia r1, {sp, lr}^
    mov r1, #0
    str r1, [sp, #84]
    mov r0, sp
    bl minemu_abort_dispatch
    ldr lr, [sp, #52]
    ldr r1, [sp, #56]
    msr spsr_cxsf, r1
    add r1, sp, #76
    ldmia r1, {sp, lr}^
    ldmia sp, {r0-r12}
    add sp, sp, #88
    subs pc, lr, #4

minemu_data_abort_entry:
    sub sp, sp, #88
    stmia sp, {r0-r12}
    str lr, [sp, #52]
    mrs r1, spsr
    str r1, [sp, #56]
    ldr r1, =-1
    str r1, [sp, #60]
    sub r1, lr, #8
    str r1, [sp, #64]
    mrc p15, 0, r1, c5, c0, 0
    str r1, [sp, #68]
    mrc p15, 0, r1, c6, c0, 0
    str r1, [sp, #72]
    add r1, sp, #76
    stmia r1, {sp, lr}^
    mov r1, #0
    str r1, [sp, #84]
    mov r0, sp
    bl minemu_abort_dispatch
    ldr lr, [sp, #52]
    ldr r1, [sp, #56]
    msr spsr_cxsf, r1
    add r1, sp, #76
    ldmia r1, {sp, lr}^
    ldmia sp, {r0-r12}
    add sp, sp, #88
    subs pc, lr, #8
