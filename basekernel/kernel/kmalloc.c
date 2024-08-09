/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kmalloc.h"
#include "console.h"
#include "kernel/types.h"
#include "memorylayout.h"

#define KUNIT sizeof(struct kmalloc_chunk)

#define KMALLOC_STATE_FREE 0xa1a1a1a1
#define KMALLOC_STATE_USED 0xbfbfbfbf


struct kmalloc_chunk {
    int state;
    int length;
    struct kmalloc_chunk *next;
    struct kmalloc_chunk *prev;
};

static struct kmalloc_chunk *head = 0;

void kmalloc_init(char *start, int length) {
    head = (struct kmalloc_chunk *)start;
    head->state = KMALLOC_STATE_FREE;
    head->length = length;
    head->next = 0;
    head->prev = 0;
}

void *kmalloc(int length) {
    length = (length + KUNIT - 1) & ~(KUNIT - 1); 
    length += KUNIT; 

    struct kmalloc_chunk *c = head;
    while (c) {
        if (c->state == KMALLOC_STATE_FREE && c->length >= length) {
            break;
        }
        c = c->next;
    }

    if (!c) {
        printf("kmalloc: out of memory!\n");
        return 0;
    }

    if (c->length - length > 2 * KUNIT) {
        struct kmalloc_chunk *n = (struct kmalloc_chunk *)((char *)c + length);
        n->state = KMALLOC_STATE_FREE; 
        n->length = c->length - length;
        n->prev = c;
        n->next = c->next;
        if (c->next) {
            c->next->prev = n;
        }
        c->next = n;
        c->length = length;
    }

    c->state = KMALLOC_STATE_USED; 
    return (c + 1);
}

void kfree(void *ptr) {
    struct kmalloc_chunk *c = (struct kmalloc_chunk *)ptr - 1;
    if (c->state != KMALLOC_STATE_USED) {
        printf("invalid kfree(%x)\n", ptr);
        return;
    }

    c->state = KMALLOC_STATE_FREE; 

    if (c->next && c->next->state == KMALLOC_STATE_FREE) {
        c->length += c->next->length;
        c->next = c->next->next;
        if (c->next) {
            c->next->prev = c;
        }
    }

    if (c->prev && c->prev->state == KMALLOC_STATE_FREE) {
        c->prev->length += c->length;
        c->prev->next = c->next;
        if (c->next) {
            c->next->prev = c->prev;
        }
        c = c->prev;
    }
}

void kmalloc_debug() {
    struct kmalloc_chunk *c;
    printf("state ptr      prev     next     length\n");
    for (c = head; c; c = c->next) {
        if (c->state == KMALLOC_STATE_FREE) {
            printf("F");
        } else if (c->state == KMALLOC_STATE_USED) {
            printf("U");
        } else {
            printf("kmalloc list corrupted at %x!\n", c);
            return;
        }
        printf("     %x %x %x %d\n", c, c->prev, c->next, c->length);
    }
}
