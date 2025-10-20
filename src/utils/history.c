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



 #include "history.h"
#include <stdio.h>
#include <string.h>

// initialize history structure to empty state
void history_init(History *history) {
    history->count = 0;
    history->capacity = MAX_HISTORY_SIZE;
    history->current_position = 0;
    history->head = 0;
    history->tail = 0;
}

// add command to end of history with duplicate filtering
void history_add(History *history, const char *command) {
    if (!command || strlen(command) == 0) return;
    
    if (history->count > 0 && 
        strcmp(history->commands[history->count - 1], command) == 0) {
        return;
    }
    
    if (history->count == history->capacity) {
        for (int i = 1; i < history->capacity; i++) {
            strcpy(history->commands[i - 1], history->commands[i]);
        }
        history->count--;
    }
    
    strncpy(history->commands[history->count], command, MAX_COMMAND_LENGTH - 1);
    history->commands[history->count][MAX_COMMAND_LENGTH - 1] = '\0';
    history->count++;
    history->current_position = history->count;
}

// remove all commands from history
void history_clear(History *history) {
    history->count = 0;
    history->current_position = 0;
}

// move position back and return previous command
const char* history_prev(History *history) {
    if (history->count == 0 || history->current_position <= 0) {
        return NULL;
    }
    history->current_position--;
    return history->commands[history->current_position];
}

// move position forward and return next command
const char* history_next(History *history) {
    if (history->current_position >= history->count - 1) {
        history->current_position = history->count;
        return "";
    }
    history->current_position++;
    return history->commands[history->current_position];
}

// reset position to end of history for new input
void history_reset_position(History *history) {
    history->current_position = history->count;
}

// retrieve command at specific index without changing position
const char* history_get_at(const History *history, int index) {
    if (index < 0 || index >= history->count) {
        return NULL;
    }
    return history->commands[index];
}

// return total number of commands stored
int history_get_count(const History *history) {
    return history->count;
}

// write all commands to file for persistence
int history_save(const History *history, const char *filepath) {
    FILE *file = fopen(filepath, "w");
    if (!file) {
        return -1;
    }
    for (int i = 0; i < history->count; i++) {
        fprintf(file, "%s\n", history->commands[i]);
    }
    fclose(file);
    return 0;
}

// load commands from file into history
int history_load(History *history, const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        return -1;
    }
    char line[MAX_COMMAND_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        history_add(history, line);
    }
    fclose(file);
    return 0;
}