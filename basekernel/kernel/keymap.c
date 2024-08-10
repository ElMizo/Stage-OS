#include "keymap.h"
#include "kernel/ascii.h"
#define KEYMAP_SHIFT 1
#define KEYMAP_ALT   2
#define KEYMAP_CTRL  3
#define KEYMAP_CAPSLOCK 4
#define KEYMAP_NUMLOCK 5
#define KEYMAP_ALPHA 6
#define KEYMAP_NUMPAD 8
#define KEYCODE_EXTRA (uint8_t)0xE0
#define KEYCODE_UP    (uint8_t)0x48
#define KEYCODE_DOWN  (uint8_t)0x42
#define KEYCODE_LEFT  (uint8_t)0x4B
#define KEYCODE_RIGHT (uint8_t)0x4D
struct keymap keymapfr[] = {
        #include "keymap.fr.pc.c"
};

struct keymap keymapus[] = {
        #include "keymap.us.pc.c"
};

struct keymap keymap[] = {
#include "keymap.us.pc.c"
};
