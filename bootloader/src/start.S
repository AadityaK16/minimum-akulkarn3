.syntax unified
.cpu cortex-a9
.arm

.section .vectors, "ax", %progbits
.global _start
.type _start, %function
_start:
    /*
     * The Cortex-A9 begins fetching at physical address zero after reset. Each
     * A32 vector slot is one instruction wide, so the first branch enters the
     * reset path and every exception that cannot be handled during boot stops
     * in a visible, deterministic loop.
     */
    b reset
    b fail_stop
    b fail_stop
    b fail_stop
    b fail_stop
    b fail_stop
    b fail_stop
    b fail_stop

.section .text.startup, "ax", %progbits
.type reset, %function
reset:
    /*
     * Enter supervisor mode with IRQ and FIQ masked. No kernel stacks or page
     * tables exist yet, so the loader uses the platform-reserved physical page
     * immediately below the boot-info record as its temporary stack.
     */
    msr cpsr_c, #0xd3
    ldr sp, =0x40007000

    /* boot_main loads the image and never returns after entering the kernel. */
    b boot_main

.global fail_stop
.type fail_stop, %function
fail_stop:
    /* Preserve the current machine state for inspection after a boot fault. */
    b fail_stop
