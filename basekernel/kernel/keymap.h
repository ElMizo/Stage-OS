#ifndef _KEYMAP_H_
#define _KEYMAP_H_

    struct keymap {
	    char normal;
	    char shifted;
	    char ctrled;
	    char special;
	    char altgr;
    };
    extern struct keymap keymapfr[128];

    extern struct keymap keymapus[128];

    extern struct keymap keymap[128];

#endif // _KEYMAP_H_
