/**
 * =====================================================
 * MemoraDB - In-Memory Database System
 * =====================================================
 * 
 * File                      : src/utils/history.c
 * Module                    : History Management
 * Last Updating Author      : Unknown
 * Last Update               : 10/20/2025
 * Version                   : 1.0.0
 * 
 * Description:
 *  Implementation of upper-arrow history functionality for MemoraDB client.
 * 
 * Copyright (c) 2025 MemoraDB Project
 * =====================================================
 */


 #define MAX_HISTORY_SIZE 100
#define MAX_COMMAND_LENGTH 1024

typedef struct History {
    char commands[MAX_HISTORY_SIZE][MAX_COMMAND_LENGTH];
    int count;
    int capacity;
    int head;               // Circular buffer head
    int tail;               // Circular buffer tail
    int current_position;   // For navigation
} History;

// initialize history structure to empty state
void history_init(History *h);

// add command to end of history with duplicate filtering
void history_add(History *h, const char *cmd);

// remove all commands from history
void history_clear(History *h);

// move position back and return previous command
const char* history_prev(History *h);

// move position forward and return next command
const char* history_next(History *h);

// reset position to end of history for new input
void history_reset_position(History *h);

// retrieve command at specific index without changing position
const char* history_get_at(const History *h, int index);

// return total number of commands stored
int history_get_count(const History *h);

// write all commands to file for persistence
int history_save(const History *h, const char *filepath);

// load commands from file into history
int history_load(History *h, const char *filepath);