#include "keymap.h"

struct keymap keymapfr[] = {
        #include "keymap.fr.pc.c"
};

struct keymap keymapus[] = {
        #include "keymap.us.pc.c"
};

struct keymap keymap[] = keymapus;