#include "commandstock.h"
#include "../kernel/kshell.h"

// Adds a new command to the command history
void add_command(const char* command)
{
    // Check if the current number of commands is less than the maximum allowed
    if (command_count < MAX_COMMANDS) 
    {
        // Duplicate the command string and store it in the command history
        command_stock[command_count] = strdup(command);

        // Increment the command count to keep track of the number of stored commands
        command_count++;

        // Update the current position to the end of the command list
        current_position = command_count; 
    } 
}

// Retrieves the previous command from the history
const char* get_previous_command()
{
    // Check if we are not already at the first command
    if (current_position > 0)
    {
        // Move one position back in the command history
        current_position--;

        // Return the previous command
        return command_stock[current_position];
    }
    // If there are no previous commands, return NULL
    return NULL; 
}

// Retrieves the next command from the history
const char* get_next_command() 
{
    // Check if we are not already at the last command
    if (current_position < command_count)
    {
        // Move one position forward in the command history
        current_position++; 

        // Return the next command
        return command_stock[current_position];
    }
    // If there are no next commands, return NULL
    return NULL; 
}