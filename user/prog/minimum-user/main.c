#include <stdint.h>

void minemu_user_main(void) {
    for (;;) {
        __asm__ volatile("nop");
    }
}
