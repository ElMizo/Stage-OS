/* ========================================================================= */
/* File Name : source/kernel/tea.c                                           */
/* File Date : 3 February 2015                                               */
/* Author(s) : Michael Collins                                               */
/* ------------------------------------------------------------------------- */
/* TEA (Tiny Encryption Algorithm)                                           */
/* ========================================================================= */

#include "../tinyos.h"
#include "../internal.h"


#define MAGIC 0x9E3779B9  // Constant used in the TEA algorithm
#define MIX   (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p%k)^e] ^ z)))  // Mix function used for encryption

int tea_encrypt(char *text, char *key) {
    unsigned int y, p, e;

    unsigned int n = kstrlen(text);  // Length of the text to be encrypted
    unsigned int k = kstrlen(key);   // Length of the encryption key
    unsigned int rounds = 6 + 64/k + 64/n;  // Number of encryption rounds
    unsigned int sum = 0;  // Sum initialized to 0
    unsigned int z = text[n-1];  // Initialize z to the last character of the text

    do {
        sum += MAGIC;  // Increment sum by MAGIC constant
        e = (sum >> 2) & 3;  // Calculate e for the MIX function
        for (p = 0; p < n-1; p++) {
            y = text[p + 1];  // Set y to the next character in the text
            z = text[p] += MIX;  // Update the current character using the MIX function
        }
        y = text[0];  // Set y to the first character in the text
        z = text[n - 1] += MIX;  // Update the last character using the MIX function
    } while (--rounds);  // Repeat for the number of rounds
    return 1;  // Return 1 to indicate successful encryption
}


int tea_decrypt(char *text, char *key) {
    unsigned int z, p, e;

    unsigned int n = kstrlen(text);  // Length of the text to be decrypted
    unsigned int k = kstrlen(key);   // Length of the decryption key
    unsigned int rounds = 6 + 64/k + 64/n;  // Number of decryption rounds
    unsigned int sum = rounds*MAGIC;  // Initialize sum to rounds * MAGIC constant
    unsigned int y = text[0];  // Initialize y to the first character of the text

    do {
        e = (sum >> 2) & 3;  // Calculate e for the MIX function
        for (p = n - 1; p > 0; p--) {
            z = text[p - 1];  // Set z to the previous character in the text
            y = text[p] -= MIX;  // Update the current character using the MIX function
        }
        z = text[n - 1];  // Set z to the last character in the text
        y = text[0] -= MIX;  // Update the first character using the MIX function
    } while ((sum -= MAGIC) != 0);  // Repeat for the number of rounds

    return 1;  // Return 1 to indicate successful decryption
}
