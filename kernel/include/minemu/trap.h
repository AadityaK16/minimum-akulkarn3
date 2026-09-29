#ifndef MINEMU_TRAP_H
#define MINEMU_TRAP_H

#include <stddef.h>
#include <stdint.h>

#define MINEMU_EXCEPTION_UNDEFINED (-4)
#define MINEMU_EXCEPTION_SVC (-3)
#define MINEMU_EXCEPTION_PREFETCH_ABORT (-2)
#define MINEMU_EXCEPTION_DATA_ABORT (-1)

struct minemu_trap_frame {
    uint32_t r[13];
    uint32_t return_lr;
    uint32_t spsr;
    int32_t exception_id;
    uint32_t fault_pc;
    uint32_t dfsr;
    uint32_t dfar;
    uint32_t user_sp;
    uint32_t user_lr;
    uint32_t reserved;
};

_Static_assert(sizeof(struct minemu_trap_frame) == 88, "trap-frame size");
_Static_assert(_Alignof(struct minemu_trap_frame) == 4, "trap-frame alignment");
_Static_assert(offsetof(struct minemu_trap_frame, return_lr) == 52, "trap-frame lr offset");
_Static_assert(offsetof(struct minemu_trap_frame, spsr) == 56, "trap-frame spsr offset");
_Static_assert(offsetof(struct minemu_trap_frame, exception_id) == 60, "trap-frame id offset");
_Static_assert(offsetof(struct minemu_trap_frame, fault_pc) == 64, "trap-frame pc offset");
_Static_assert(offsetof(struct minemu_trap_frame, user_sp) == 76, "trap-frame user sp offset");

void minemu_undefined_dispatch(struct minemu_trap_frame *frame);
void minemu_abort_dispatch(struct minemu_trap_frame *frame);
void minemu_fail_stop(void) __attribute__((noreturn));

#endif
