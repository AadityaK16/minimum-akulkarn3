#include "minemu/platform.h"
#include "minemu/trace.h"

void minemu_trace_event(uint32_t value) {
    MINEMU_TRACE->event = value;
}
