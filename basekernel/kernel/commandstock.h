#ifndef COMMANDSTOCK_H
#define COMMANDSTOCK_H

#define MAX_COMMANDS 100
#define COMMAND_LENGTH 256
#define NULL 0

char* command_stock[MAX_COMMANDS];
int command_count =  0 ; 
int current_position = -1 ; 


void add_command(const char* command); 
const char* get_previous_command();
const char* get_next_command();


#endif

