#include <PR/libaudio.h>

void alUnlink(ALLink *ln) {
    if (ln->next) {
        ln->next->prev = ln->prev;
    }

    if (ln->prev) {
        ln->prev->next = ln->next;
    }
}

void alLink(ALLink *ln, ALLink *to) {
    ln->next = to->next;
    ln->prev = to;

    if (to->next) {
        to->next->prev = ln;
    }

    to->next = ln;
}
