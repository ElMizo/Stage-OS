/* ========================================================================= */
/* File Name : source/kernel/kernel.c                                        */
/* File Date : 3 February 2015                                               */
/* Author(s) : Michael Collins                                               */
/* ------------------------------------------------------------------------- */
/* 32bit protected mode unix like kernel entry point.                        */
/* ========================================================================= */

#include "../tinyos.h"
#include "../internal.h"


#ifdef  NDEBUG
#undef  main                 // Undefine main if NDEBUG is defined
#define main mainCRTStartup  // Redefine main as mainCRTStartup
#endif


void main(void) {
    // If system initialization fails, hang indefinitely
    if (!init()) for (;;);

    // Infinite loop to handle user logins and shell interactions
    for (;;) {
        USER *user = logon();  // Attempt to log in a user
        if (user) {            // If login is successful
            shell_main(user);  // Start the shell for the logged-in user
            kfree(user, sizeof(USER));  // Free memory allocated for the user structure
        }
    }
}
