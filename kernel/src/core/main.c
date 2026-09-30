#include <stdint.h>
#include <stdbool.h>
#include "minemu/boot.h"
#include "minemu/trap.h"
#include "minemu/trace.h"
#include "minemu/platform.h"
#include "minemu/irq.h"

/*
 * Forward declarations for functions defined in uart.c
 * These tell the compiler these functions exist elsewhere
 * and will be linked in at link time.
 */
void uart_putc(char c);
void uart_puts(const char *s);
char uart_getc(void);
void uart_irq_handler(void);

/*
 * IRQ handler function table.
 * Instead of a chain of if/else, we use an array of function pointers
 * indexed by IRQ source ID. This scales cleanly as we add more devices
 * later (SysTick for scheduling, DMA, etc.) without touching dispatch logic.
 *
 * 32 entries covers all possible source IDs on this platform.
 * Unregistered slots are NULL and safely skipped in the dispatcher.
 *
 * Future: when adding SysTick for scheduling, just do:
 *   irq_handlers[MINEMU_IRQ_SYSTICK] = systick_handler;
 */
static void (*irq_handlers[32])(void) = {0};

/*
 * minemu_irq_dispatch - called from the assembly trampoline on every IRQ
 *
 * Receives a pointer to the trap frame (saved CPU state at time of interrupt).
 * Looks up and calls the registered handler for the interrupt source,
 * then writes EOI to tell the interrupt controller we are done.
 * Returns the frame pointer - for now always the same frame,
 * but later when scheduling is added, this may return a different
 * frame to switch to a different task.
 *
 * Note: EOI is written AFTER the handler returns. This is important
 * for UART because the handler drains the hardware buffer first,
 * clearing the interrupt condition before we signal completion.
 */
struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;

    /* call the registered handler if one exists for this source */
    if (source < 32 && irq_handlers[source]) {
        irq_handlers[source]();
    }

    /* signal end of interrupt to the controller */
    MINEMU_INTERRUPT->eoi = source;

    /* return same frame for now - scheduler will change this later */
    return frame;
}

/*
 * Maximum line length for msh input.
 * Lines longer than this are considered overflow and discarded.
 * 20 bytes as specified in the assignment.
 */
#define LINE_MAX 20

/*
 * run_msh - the minimum shell main loop
 *
 * Runs forever, reading lines from UART via uart_getc() and
 * dispatching to command handlers.
 *
 * Design decisions:
 * - Line buffer is stack-allocated (simple, no heap needed)
 * - Overflow flag discards the whole line if it exceeds LINE_MAX
 * - Both 0x08 and 0x7f treated as backspace per assignment spec
 * - Leading and repeated spaces handled explicitly
 *
 * Future: when user processes are added, msh will become a user
 * program that talks to the kernel via syscalls instead of
 * directly calling uart functions.
 */
static void run_msh(void) {
    char line[LINE_MAX];     /* not null-terminated; len tracks the valid bytes */
    int len = 0;             /* current number of chars in line buffer */
    bool overflow = false;   /* true if line exceeded LINE_MAX */

    /* Task 1: boot banner, printed once after boot-info validation */
    uart_puts("hello world\n");

    uart_puts("msh> ");

    for (;;) {
        /* blocking read - waits for IRQ handler to put a byte in the buffer */
        char c = uart_getc();

        /*
         * Handle backspace: 0x08 (BS) and 0x7f (DEL) both mean backspace.
         * If line is empty, ignore. Otherwise remove last character.
         * The TUI mirrors backspace automatically so no erase sequence needed.
         */
        if (c == 0x08 || c == 0x7f) {
            if (len > 0) len--;
            continue;
        }

        /*
         * Line terminator. Per the spec, only '\n' ends a line;
         * '\r' is treated as an ordinary character.
         */
        
        if (c == '\n') {
            uart_putc('\n');

            if (!overflow) {
                /* skip leading spaces */
                int i = 0;
                while (i < len && line[i] == ' ') i++;

                if (i == len) {
                    /*
                     * Empty line or all spaces: do nothing, just reprint prompt.
                     * Assignment says treat as empty command.
                     */
                } else if (len - i >= 4 &&
                           line[i]   == 'e' &&
                           line[i+1] == 'c' &&
                           line[i+2] == 'h' &&
                           line[i+3] == 'o' &&
                           (i + 4 == len || line[i+4] == ' ')) {
                    /*
                     * echo command: print everything after "echo",
                     * skipping the single separator space (and any repeated spaces).
                     * echo with no argument prints an empty line.
                     */
                    int j = i + 4; /* skip past "echo" */
                    while (j < len && line[j] == ' ') j++; /* skip spaces */
                    for (int k = j; k < len; k++) uart_putc(line[k]);
                    uart_putc('\n');
                } else {
                    /*
                     * Unknown command: print "command not found: COMMAND"
                     * where COMMAND is the first space-delimited word.
                     */
                    uart_puts("command not found: ");
                    int j = i;
                    while (j < len && line[j] != ' ') uart_putc(line[j++]);
                    uart_putc('\n');
                }
            }
            /* overflow lines are silently discarded - just reprint prompt */

            /* reset line buffer for next command */
            len = 0;
            overflow = false;
            uart_puts("msh> ");
            continue;
        }

        /*
         * Normal character: add to line buffer if space available.
         * If buffer is full, set overflow flag and discard character.
         * We continue reading until newline so the user can finish typing.
         */
        if (len < LINE_MAX) {
            line[len++] = c;
        } else {
            overflow = true;
        }
    }
}

/*
 * minemu_kernel_main - kernel entry point, called by bootloader
 *
 * Validates boot info, sets up UART interrupts, enables IRQs,
 * then enters the shell loop. Never returns.
 */
void minemu_kernel_main(const struct minemu_boot_info *boot_info) {
    /*
     * Validate boot info struct passed by bootloader.
     * If any field is wrong, fire a trace event and halt.
     * Do not modify this block - required by assignment.
     */
    if ((uintptr_t)boot_info != MINEMU_BOOT_INFO_VADDR ||
            boot_info->magic != MINEMU_BOOT_INFO_MAGIC ||
            boot_info->version != MINEMU_ABI_VERSION ||
            boot_info->size != sizeof(*boot_info) ||
            boot_info->system_rom_base != UINT32_C(0x08000000) ||
            boot_info->direct_map_vaddr != UINT32_C(0xc0000000) ||
            boot_info->direct_map_paddr != UINT32_C(0x40000000) ||
            boot_info->direct_map_size != UINT32_C(0x04000000)) {
        minemu_trace_event(UINT32_C(0xb007bad0));
        minemu_fail_stop();
    }

    /*
     * Register UART0 interrupt handler in the function table.
     * This must happen before enabling interrupts, otherwise
     * an interrupt could fire before the handler is registered.
     */
    irq_handlers[MINEMU_IRQ_UART0] = uart_irq_handler;

    /* Only one IRQ source is enabled, so priority is irrelevant for now. */
    MINEMU_INTERRUPT->priority_uart0 = 1;
    /*
     * Enable UART0 RX interrupt generation.
     * This tells the UART device to fire an IRQ when data arrives.
     * Without this, the UART device stays silent even if data arrives.
     */
    MINEMU_UART0->control = MINEMU_UART_CONTROL_RX_IRQ_ENABLE;

    /*
     * Enable UART0 in the interrupt controller.
     * The interrupt controller acts as a gatekeeper - even if UART
     * fires an interrupt, the controller blocks it unless enabled here.
     */
    MINEMU_INTERRUPT->enable = UINT32_C(1) << MINEMU_IRQ_UART0;

    /*
     * Enable IRQs on the CPU.
     * cpsie i = Change Processor State, Interrupt Enable.
     * Until this point interrupts were masked by the bootloader.
     * The "memory" clobber prevents the compiler from reordering
     * memory accesses across this barrier.
     */
    __asm__ volatile("cpsie i" : : : "memory");

    /* enter the shell - never returns */
    run_msh();
}