#include "utils.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// print to stderr
void eprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);

    va_end(args);
}

void unreachable(const char* errmsg) {
    assert(errmsg != NULL);
    eprintf("This should never happend %s", errmsg);
    exit(RET_CODE_ERROR);
}
