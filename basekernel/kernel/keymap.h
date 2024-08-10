#ifndef _KEYMAP_H_
#define _KEYMAP_H_

    struct keymap {
	    char normal;
	    char shifted;
	    char ctrled;
	    char special;
	    char altgr;
    };
    extern struct keymap keymapfr[];

    extern struct keymap keymapus[];

    extern struct keymap keymap[];

#endif // _KEYMAP_H_