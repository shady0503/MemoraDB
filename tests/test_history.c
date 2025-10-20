/**
 * =====================================================
 * MemoraDB - In-Memory Database System
 * =====================================================
 * 
 * File                      : tests/test_history.c
 * Module                    : Command History Unit Tests
 * Last Updating Author      : Unknown
 * Last Update               : 10/20/2025
 * Version                   : 1.0.0
 *
 * Description:
 *  Simple unit tests for command history operations.
 * 
 * Copyright (c) 2025 MemoraDB Project
 * =====================================================
 */

#include <assert.h>
#include <string.h>
#include "../src/utils/history.h"
#include "test_framework.h"

void test_history_basic() {
    printf("Testing basic history operations...\n");
    
    History history;
    history_init(&history);
    
    history_add(&history, "PING");
    history_add(&history, "SET key value");
    history_add(&history, "GET key");
    
    TEST_ASSERT(history.count == 3, "Should have 3 commands");
    TEST_ASSERT(strcmp(history.commands[0], "PING") == 0, "First command correct");
    
    TEST_SUCCESS("Basic history test passed");
}

void test_history_navigation() {
    printf("Testing history navigation...\n");
    
    History history;
    history_init(&history);
    
    history_add(&history, "first");
    history_add(&history, "second");
    
    const char *cmd = history_prev(&history);
    TEST_ASSERT(strcmp(cmd, "second") == 0, "Should get last command");
    
    cmd = history_prev(&history);
    TEST_ASSERT(strcmp(cmd, "first") == 0, "Should get first command");
    
    cmd = history_next(&history);
    TEST_ASSERT(strcmp(cmd, "second") == 0, "Should go forward");
    
    TEST_SUCCESS("Navigation test passed");
}

void test_history_duplicates() {
    printf("Testing duplicate filtering...\n");
    
    History history;
    history_init(&history);
    
    history_add(&history, "PING");
    history_add(&history, "PING");
    history_add(&history, "PING");
    
    TEST_ASSERT(history.count == 1, "Duplicates should be filtered");
    
    TEST_SUCCESS("Duplicate filtering test passed");
}

int main() {
    init_test_framework();
    printf("=== Command History Tests ===\n");
    
    test_history_basic();
    test_history_navigation();
    test_history_duplicates();
    
    save_test_results();
    return total_tests_failed > 0 ? 1 : 0;
}