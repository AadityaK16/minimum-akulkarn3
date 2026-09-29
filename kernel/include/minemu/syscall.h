#ifndef MINEMU_SYSCALL_H
#define MINEMU_SYSCALL_H

#include "minemu/trap.h"

/* Students implement this A32 vector trampoline and its syscall contract. */
void minemu_svc_trampoline(void) __attribute__((noreturn));
struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame);

#endif
