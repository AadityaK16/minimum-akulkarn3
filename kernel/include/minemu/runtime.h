#ifndef MINEMU_RUNTIME_H
#define MINEMU_RUNTIME_H

#include "minemu/trap.h"

void minemu_panic(const char *message) __attribute__((noreturn));

#endif
