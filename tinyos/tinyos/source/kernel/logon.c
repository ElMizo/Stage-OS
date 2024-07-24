/* ========================================================================= */
/* File Name : source/kernel/logon.c                                         */
/* File Date : 3 February 2015                                               */
/* Author(s) : Michael Collins                                               */
/* ------------------------------------------------------------------------- */
/* user logon routine. Uses TEA (Tiny Encryption Algorithm) for checking the */
/* password. We do not store the password directly, but rather we encrypt    */
/* the username with the password. If the encrypted username matches the one */
/* we associate with the user, then we can assume that the correct password  */
/* was enterred.                                                             */
/* ========================================================================= */

#include "../tinyos.h"    // Include the header file for tinyOS definitions and functions
#include "../internal.h"  // Include the header file for internal type definitions

/* logon: "root"         */
/* password: ""          */
/*                       */
/* logon: "mike"         */
/* password: "letmein"   */

int curuser = -1;  // Variable to store the index of the currently logged-in user, initialized to -1 (no user logged in)
USER users[256] = {  // Array to store information for up to 256 users
    /* name    pass     home    userid     */
    { "root", "Ç–~4w", "/root", 0x00000001 },  // User "root" with encrypted password "Ç–~4w", home directory "/root", and user ID 1
    { "mike", "áÚÀ> ", "/mike", 0x00000002 },  // User "mike" with encrypted password "áÚÀ> ", home directory "/mike", and user ID 2
};


USER *logon(void) {
    char user[256];  // Buffer to store the input username
    char pass[256];  // Buffer to store the input password
    char hash[256];  // Buffer to store the encrypted username
    int  i;          // Loop counter

    FILE *terminal = kopen("tty", "rw");  // Open the terminal device for reading and writing
    if (!terminal) return 0;  // If the terminal could not be opened, return 0 (failure)

    kmemset(user, 0, sizeof(user));  // Clear the user buffer
    kmemset(pass, 0, sizeof(pass));  // Clear the pass buffer
    kmemset(hash, 0, sizeof(hash));  // Clear the hash buffer

    kfprintf(stdout, "logon: ");  // Print "logon: " to the standard output
    kfgets(stdin, user, sizeof(user));  // Read the input username from the standard input

    kfprintf(stdout, "password: ");  // Print "password: " to the standard output
    kfgets(stdin, pass, sizeof(pass));  // Read the input password from the standard input

    curuser = -1;  // Reset the current user index
    for (i = 0; i < elementsof(users); i++) {  // Loop through all the users in the users array
        if (kmemcmp(user, users[i].name, kstrlen(users[i].name)+1) == 0) {  // Compare the input username with the username stored in the users array
            USER *usr;  // Pointer to store the matched user

            kmemcpy(hash, user, sizeof(hash));  // Copy the input username to the hash buffer
            tea_encrypt(hash, pass);  // Encrypt the hash buffer (username) with the input password
            if (kstrcmp(hash, users[i].pass) != 0) break;  // If the encrypted username does not match the stored password, break the loop

            usr = (USER*)kalloc(sizeof(USER));  // Allocate memory for a new USER structure
            if (!usr) return 0;  // If memory allocation fails, return 0 (failure)

            kmemcpy(usr, &users[i], sizeof(USER));  // Copy the matched user data to the allocated USER structure
            curuser = i;  // Set the current user index to the matched user's index
            return usr;  // Return the pointer to the matched user
        }
    }
    return 0;  // If no matching user is found, return 0 (failure)
}


USER *kgetuser(void) {
    USER *usr;  // Pointer to store the current user

    if (curuser < 0 || curuser >= elementsof(users)) return 0;  // If no user is logged in or the current user index is out of bounds, return 0 (failure)

    usr = (USER*)kalloc(sizeof(USER));  // Allocate memory for a new USER structure
    if (!usr) return 0;  // If memory allocation fails, return 0 (failure)

    kmemcpy(usr, &users[curuser], sizeof(USER));  // Copy the current user data to the allocated USER structure
    usr->pass = 0;  // Set the pass field to null to avoid exposing the password
    return usr;  // Return the pointer to the current user
}

