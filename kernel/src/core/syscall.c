#include <stdint.h>
#include "minemu/syscall.h" // declares minemu svc dispatch
#include "minemu/trap.h" //struct minemu_trap_frame
#include "minemu/user_abi.h" // MINEMU_SYSCALL_IOCTL, MINEMU_FD*, MINEMU_IOCTL_*
#include "minemu/platform.h" //MINEMU_UART0, MINEMU_RNG, MINEMU_TRACE
#include "minemu/mmu.h" //MINEMU_PTE_READABLE  and MINEMU_PTE_WRITABLE
#include "loader.h" // user_range_ok


#define CPSR_MODE_MASK UINT32_C(0x1f)
#define CPSR_MODE_USR UINT32_C(0x10)
#define CPSR_THUMB UINT32_C(0x20)
#define A32_SVC_ZERO UINT32_C(0xef000000)

// from uart.c, we get the non-blocking dequeue from UART0 RX ring buffer
int uart_try_getc(char *out);

// UART

static int32_t uart_write(volatile struct minemu_uart_regs *uart, uint32_t src, uint32_t n){

    if( n > MINEMU_IOCTL_MAX_TRANSFER || !user_range_ok(src, n, MINEMU_PTE_READABLE)){
        return MINEMU_IOCTL_ERROR;
    }

    const char *p = (const char *)(uintptr_t) src;

    for (uint32_t i = 0; i < n; i++){
        uart-> tx_data = (uint32_t)(uint8_t)p[i];
    }

    return (int32_t)n;
}


static int32_t uart_read(volatile struct minemu_uart_regs *uart, uint32_t dst, uint32_t n){
    if(n > MINEMU_IOCTL_MAX_TRANSFER){
        n = MINEMU_IOCTL_MAX_TRANSFER;
    }

    // validating before consuming any input

    if(!user_range_ok(dst, n, MINEMU_PTE_WRITABLE)){
        return MINEMU_IOCTL_ERROR;
    }

    char *p = (char *)(uintptr_t) dst;
    uint32_t count = 0;

    if(uart == MINEMU_UART0){

        //UART0 imput arrives through the IRQ-filled ring buffer.
        char c;
        while(count < n && uart_try_getc(&c)){
            p[count++] = c;
        }
    }

    else{
        // UART1 has no IRQ buffer, therefore polling the device.

        while(count < n && (uart->status & MINEMU_UART_STATUS_RX_READY) != 0){
            p[count++] = (char)(uart->rx_data & 0xffU);
        }
    }

    return (int32_t) count;
}

// RNG
static int32_t rng_op(uint32_t request, uint32_t arg0){
        switch (request){
        case MINEMU_IOCTL_RNG_SEED:
            MINEMU_RNG->seed = arg0;
            return 0;
        
        case MINEMU_IOCTL_RNG_STATE:
        case MINEMU_IOCTL_RNG_NEXT:

        // validating before reading as reading data advances RNG

            if((arg0 & 3U) != 0 || !user_range_ok(arg0, 4, MINEMU_PTE_WRITABLE)){
                return MINEMU_IOCTL_ERROR;
            }

            *(uint32_t *)(uintptr_t)arg0 = (request == MINEMU_IOCTL_RNG_STATE) ? MINEMU_RNG->state : MINEMU_RNG->data;

            return 0;
        default:
            return MINEMU_IOCTL_ERROR;
        }
}

// Trace
static int32_t trace_op(uint32_t request, uint32_t arg0){

    if(request != MINEMU_IOCTL_TRACE_EVENT){
        return MINEMU_IOCTL_ERROR;
    }

    MINEMU_TRACE->event = arg0;
    return 0;
}

// IOTC router
static int32_t sys_IOCTL(uint32_t fd, uint32_t request, uint32_t arg0, uint32_t arg1){

    volatile struct minemu_uart_regs *uart;
    /* fds follow minemu/user_abi.h (used by the tests), not the MMIO addresses
   in the spec table. UART_IN/UART_OUT -> UART0, UART_DIAG -> UART1. */
    switch(fd){

        case MINEMU_FD_UART_IN:
        case MINEMU_FD_UART_OUT:
            uart = MINEMU_UART0;
            break;
        case MINEMU_FD_UART_DIAG:
            uart = MINEMU_UART1;
            break;
        case MINEMU_FD_RNG:
            return rng_op(request, arg0);
        case MINEMU_FD_TRACE:
            return trace_op(request, arg0);
        default:
            return MINEMU_IOCTL_ERROR;
    }

    switch(request){

        case MINEMU_IOCTL_UART_WRITE:
            return uart_write(uart, arg0, arg1);
        case MINEMU_IOCTL_UART_READ:
            return uart_read(uart, arg0, arg1);
        default:
            return MINEMU_IOCTL_ERROR;
    }
}



// dispatcher (called form svc.S)

struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame){

    // The frame must be an SVC taken from A32 user mode

    if((frame->exception_id != MINEMU_EXCEPTION_SVC) || ((frame->spsr & CPSR_MODE_MASK) != CPSR_MODE_USR) || ((frame->spsr & CPSR_THUMB) != 0)){
        frame->r[0] = (uint32_t)MINEMU_IOCTL_ERROR;
        return frame;
    }

    // The trap instruction must be exactly svc 0 

    if((!user_range_ok(frame->fault_pc, 4, MINEMU_PTE_READABLE) || *(const uint32_t *)(uintptr_t)frame->fault_pc != A32_SVC_ZERO)){
        frame->r[0] = (uint32_t)MINEMU_IOCTL_ERROR;
        return frame;
    }

    // syscall number should be in r7 adn the args in r0-r3

    int32_t result;
    switch(frame->r[7]){
        case MINEMU_SYSCALL_IOCTL:
            result = sys_IOCTL(frame->r[0], frame->r[1], frame->r[2], frame->r[3]);
            break;
        default:
            result = MINEMU_IOCTL_ERROR;
            break;
    }

    // Return value goes back to the user in r0

    frame->r[0] = (uint32_t) result;
    return frame;
}