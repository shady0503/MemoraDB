/**
 * =====================================================
 * MemoraDB - In-Memory Database System
 * =====================================================
 * 
 * File                      : src/client/client.c
 * Module                    : Client Utilities
 * Last Updating Author      : Unknown
 * Last Update               : 10/20/2025
 * Version                   : 1.0.0
 * 
 * Description:
 *  Simple client to test the MemoraDB server.
 * 
 * 
 * Copyright (c) 2025 MemoraDB Project
 * =====================================================
 */

#include "client.h"
#include "resp_parser.h"
#include "../utils/logo.h"
#include "../utils/history.h"
#include <termios.h>
#include <ctype.h>

static int enable_raw_mode(struct termios *orig) {
    struct termios raw;
    if (tcgetattr(0, orig) == -1) return -1;
    raw = *orig;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSAFLUSH, &raw) == -1) return -1;
    return 0;
}

static void disable_raw_mode(struct termios *orig) {
    tcsetattr(0, TCSAFLUSH, orig);
}

static int read_line_with_history(History *h, char *buf, int maxlen) {
    struct termios orig;
    char tmp[1024] = {0};
    int pos = 0;

    if (enable_raw_mode(&orig) == -1) {
        /* Fallback to standard fgets if raw mode not available */
        if (fgets(buf, maxlen, stdin) == NULL) return -1;
        buf[strcspn(buf, "\n")] = 0;
        return (int)strlen(buf);
    }

    history_reset_position(h);

    while (1) {
        char c;
        if (read(0, &c, 1) != 1) break;

        /* Handle escape sequences for arrow keys */
        if (c == 27) {
            char seq[2];
            if (read(0, &seq[0], 1) != 1) continue;
            if (read(0, &seq[1], 1) != 1) continue;
            if (seq[0] == '[') {
                if (seq[1] == 'A') { /* Up */
                    const char *v = history_prev(h);
                    if (v) {
                        printf("\r\033[KMemoraDB> %s", v);
                        fflush(stdout);
                        strncpy(tmp, v, sizeof(tmp) - 1);
                        tmp[sizeof(tmp) - 1] = '\0';
                        pos = (int)strlen(tmp);
                    }
                } else if (seq[1] == 'B') { /* Down */
                    const char *v = history_next(h);
                    if (v) {
                        printf("\r\033[KMemoraDB> %s", v);
                        fflush(stdout);
                        strncpy(tmp, v, sizeof(tmp) - 1);
                        tmp[sizeof(tmp) - 1] = '\0';
                        pos = (int)strlen(tmp);
                    }
                }
            }
            continue;
        }

        /* Backspace */
        if (c == 127 || c == 8) {
            if (pos > 0) {
                pos--;
                tmp[pos] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        /* Enter */
        if (c == '\n' || c == '\r') {
            printf("\n");
            break;
        }

        /* Ctrl-C */
        if (c == 3) {
            printf("^C\n");
            disable_raw_mode(&orig);
            return -1;
        }

        /* Printable characters */
        if (isprint((unsigned char)c) && pos < maxlen - 1 && pos < (int)sizeof(tmp) - 1) {
            tmp[pos++] = c;
            tmp[pos] = '\0';
            printf("%c", c);
            fflush(stdout);
        }
    }

    disable_raw_mode(&orig);
    strncpy(buf, tmp, maxlen - 1);
    buf[maxlen - 1] = '\0';
    return pos;
}
// ================================================

int main(int argc, char *argv[]) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char command[BUFFER_SIZE];
    const char *server_ip = "127.0.0.1";
    
    History history;history_init(&history); // ADD THIS LINE
    
    //-- Override server IP if provided as argument --//
    if (argc > 1) {
        server_ip = argv[1];
    }
    
    display_memoradb_logo();
    printf("\n\n");
    
    printf("===============================================\n");
    printf("     MemoraDB Client - Testing Interface      \n");
    printf("        Connecting to: %s:%d               \n", server_ip, SERVER_PORT);
    printf("===============================================\n");
    
    //-- Create socket --//
    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        printf("[Client: ERROR] Socket creation failed: %s\n", strerror(errno));
        return 1;
    }
    
    printf("[Client: INFO] Socket created successfully.\n");
    
    //-- Configure server address --//
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        printf("[Client: ERROR] Invalid address: %s\n", server_ip);
        close(client_fd);
        return 1;
    }

    //-- Connect to server --//
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("[Client: ERROR] Connection failed: %s\n", strerror(errno));
        close(client_fd);
        return 1;
    }

    printf("[Client: INFO] Connected to MemoraDB server\n\n");
    printf("[Client: INFO] Type commands (EXIT or QUIT to close the connection):\n");
    printf("=======================================================================\n");

    //-- Interactive command loop --//
    while (1) {
        printf("MemoraDB> ");
        fflush(stdout);
        memset(command, 0, sizeof(command));

        // REPLACE THIS LINE:
        // if (fgets(command, sizeof(command), stdin) == NULL) {
        // WITH THIS:
        if (read_line_with_history(&history,command,sizeof(command))<0) {
            break;
        }
        
        //-- Remove newline --//
        command[strcspn(command, "\n")] = 0;
        
        if (strcasecmp(command, "quit") == 0 || strcasecmp(command, "exit") == 0) {
            printf("[Client: INFO] Disconnecting...\n");
            break;
        }
        
        //-- Skip empty commands --//
        if (strlen(command) == 0) {
            continue;
        }
        
        history_add(&history,command); // ADD THIS LINE
        
        char * argv[BUFFER_SIZE];
        int argc = 0;

        char * token = strtok(command, " ");
        while(token != NULL && argc < BUFFER_SIZE){
            argv[argc++] = token;
            token = strtok(NULL, " ");
        }

        int offset = 0;
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "*%d\r\n", argc);
        for(int i = 0; i < argc; i++){
            offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "$%lu\r\n%s\r\n", strlen(argv[i]), argv[i]);
        }

        if (send(client_fd, buffer, offset, 0) < 0) {
            printf("[Client: ERROR] Failed to send command: %s\n", strerror(errno));
            break;
        }
        
        //-- Receive response --//
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received < 0) {
            printf("[Client: ERROR] Failed to receive response: %s\n", strerror(errno));
            break;
        } else if (bytes_received == 0) {
            printf("[Client: INFO] Server closed connection.\n");
            break;
        }

        //-- Display response --//
        buffer[bytes_received] = '\0';
        
        //-- Check if response starts with RESP protocol markers --//
        if (buffer[0] == '+' || buffer[0] == '-' || buffer[0] == ':' || 
            buffer[0] == '$' || buffer[0] == '*') {
            //-- Parse and display RESP response --//
            int parsed = parse_and_display_resp(buffer);
            if (parsed == -1) {
                printf("Error parsing RESP response\n");
                printf("Raw response: %s", buffer);
            }
        } else {
            //-- Display raw response (for MemoraDB logging messages) --//
            printf("%s", buffer);
        }
        
        command[strlen(command) - 2] = '\0';
    }
    
    close(client_fd);
    printf("[Client: INFO] Connection closed.\n");
    return 0;
}