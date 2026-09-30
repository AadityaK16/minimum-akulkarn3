#include <stdint.h>
#include "minemu/platform.h"
#include "minemu/irq.h"

/*
 * UART Driver for minemu UART0
 *
 * Design: interrupt-driven receive, polling transmit.
 * - TX (sending): we poll the status register until ready, then write.
 *   This is fine because we control when we send and sends are fast.
 * - RX (receiving): we use interrupts so the CPU isn't wasting cycles
 *   waiting for the user to type. When a byte arrives, the IRQ fires
 *   and we store it in a ring buffer. The shell reads from the buffer
 *   whenever it's ready.
 */

/*
 * Ring buffer for incoming UART bytes.
 * A ring buffer uses two indices into a fixed array:
 *   head = where the IRQ handler writes next incoming byte
 *   tail = where the shell reads the next byte from
 * When head == tail, the buffer is empty.
 * When (head+1) % SIZE == tail, the buffer is full.
 * volatile tells the compiler these can change outside normal code flow
 * (i.e. from an interrupt), so don't optimize away reads/writes to them.
 *
 * Future: when scheduling is added, replace this with a proper kernel
 * wait queue so the shell can sleep instead of busy-waiting.
 */

#define UART_BUF_SIZE 256
static volatile char uart_buf[UART_BUF_SIZE];
static volatile uint32_t uart_buf_head = 0;  /* IRQ writes here */
static volatile uint32_t uart_buf_tail = 0;  /* shell reads here */

/*
 * uart_putc - transmit one byte over UART0
 *
 * Writes directly to tx_data without polling TX ready status.
 * The hardware TX buffer is 8192 bytes which is large enough
 * that we never overflow it for small shell output.
 *
 * MMIO rule: each access must be a single aligned 32-bit read or write.
 * That's why tx_data takes a uint32_t even though we're sending one byte.
 */
void uart_putc(char c) {
    /* write the byte - only low 8 bits are used by hardware */
    MINEMU_UART0->tx_data = (uint32_t)c;
}

/*
 * uart_puts - transmit a null-terminated string over UART0
 *
 * Simply calls uart_putc for each character until the null terminator.
 */
void uart_puts(const char *s) {
    while (*s) uart_putc(*s++);
}



/*
 * uart_irq_handler - called from IRQ dispatcher when UART0 fires
 *
 * UART RX interrupts stay asserted as long as there are unread bytes
 * in the hardware's internal buffer. So we must drain ALL available
 * bytes in one handler call, otherwise the interrupt immediately
 * re-triggers after EOI.
 *
 * For each byte:
 *   - compute next_head to check if buffer is full before writing
 *   - if full, drop the byte (silent drop - a real driver might set
 *     an overflow flag, but that's out of scope here)
 *   - if not full, store the byte and advance head
 *
 * Note: we do NOT write EOI here. That's the dispatcher's job after
 * this handler returns, ensuring proper interrupt lifecycle management.
 */
void uart_irq_handler(void) {
    /* keep reading while hardware RX buffer has data */
    while (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) {

        /* read one byte from hardware - this consumes it from HW buffer */
        
        char c = (char)(MINEMU_UART0->rx_data & 0xFF);

        /* check if our ring buffer has space before writing */
        uint32_t next_head = (uart_buf_head + 1) % UART_BUF_SIZE;
        if (next_head != uart_buf_tail) {

            /* space available: store byte and advance head */
            
            uart_buf[uart_buf_head] = c;
            uart_buf_head = next_head;
        }
        /* if next_head == tail, buffer is full: byte is silently dropped */
    }
}

/*
 * uart_getc - read one byte from the ring buffer (blocking)
 *
 * Spins until a byte is available (head != tail), then reads it.
 *
 * Critical section: we disable interrupts around the actual buffer
 * read and tail update. This prevents a race condition where the IRQ
 * fires between reading the byte and updating tail, potentially
 * corrupting the buffer state.
 *
 * cpsid i = disable IRQ interrupts (ARM instruction)
 * cpsie i = re-enable IRQ interrupts
 *
 * Future: replace the spin loop with a scheduler sleep call once
 * scheduling is implemented, so the CPU can do other work while waiting.
 */

char uart_getc(void) {

    /* busy-wait until IRQ handler has put something in the buffer */
    while (uart_buf_head == uart_buf_tail){

        if (MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY){
            uart_irq_handler();
        }
    }
    /* begin critical section: disable interrupts */
    __asm__ volatile("cpsid i" : : : "memory");

    char c = uart_buf[uart_buf_tail];
    /* advance tail, wrapping around at UART_BUF_SIZE */
    uart_buf_tail = (uart_buf_tail + 1) % UART_BUF_SIZE;

    /* end critical section: re-enable interrupts */
    __asm__ volatile("cpsie i" : : : "memory");

    return c;
}
