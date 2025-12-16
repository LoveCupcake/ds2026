#include "db.h"
#include "distributed.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <port> [peer_port1] [peer_port2] ...\n", argv[0]);
        return 1;
    }

    int my_port = atoi(argv[1]);
    
    // Open database (creates node.wal if doesn't exist)
    char walfile[64];
    snprintf(walfile, sizeof(walfile), "node_%d.wal", my_port);
    DB *db = db_open(walfile);
    
    if (!db) {
        printf("Failed to open database\n");
        return 1;
    }

    // Parse peer list
    for (int i = 2; i < argc && peer_count < MAX_PEERS; i++) {
        strcpy(peers[peer_count].ip, "127.0.0.1");
        peers[peer_count].port = atoi(argv[i]);
        peer_count++;
    }

    // Create server socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    
    // Allow port reuse
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(my_port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }
    
    if (listen(sock, 10) < 0) {
        perror("listen");
        close(sock);
        return 1;
    }

    printf("=================================\n");
    printf("Node running on port %d\n", my_port);
    printf("WAL file: %s\n", walfile);
    printf("Peers: %d\n", peer_count);
    for (int i = 0; i < peer_count; i++) {
        printf("  - %s:%d\n", peers[i].ip, peers[i].port);
    }
    printf("=================================\n");

    while (1) {
        int c = accept(sock, NULL, NULL);
        if (c < 0) {
            perror("accept");
            continue;
        }
        
        char buf[1024] = {0};
        char key[MAX_KEY] = {0};
        char value[MAX_VALUE] = {0};

        ssize_t n = recv(c, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(c);
            continue;
        }
        buf[n] = '\0';

        printf("\n[REQUEST] %s", buf);

        if (sscanf(buf, "PUT %127s %511s[^\n]", key, value) == 2) {
            char *val = value;
            while (*val == ' ') val++;
            
            printf("[PUT] key=%s value=%s\n", key, value);
            dist_put(key, value);
            const char *response = "OK\n";
            send(c, response, strlen(response), 0);
            
        } else if (sscanf(buf, "GET %127s", key) == 1) {
            printf("[GET] key=%s\n", key);
            if (db_get(db, key, value)) {
                printf("[GET] Found: %s\n", value);
                send(c, value, strlen(value), 0);
                send(c, "\n", 1, 0);
            } else {
                printf("[GET] Not found\n");
                const char *response = "NOT_FOUND\n";
                send(c, response, strlen(response), 0);
            }
            
        } else if (sscanf(buf, "REPL %127s %511s", key, value) == 2) {
            printf("[REPL] Received: key=%s value=%s\n", key, value);
            db_put(db, key, value);
            
        } else {
            printf("[ERROR] Invalid command: %s\n", buf);
            const char *response = "ERROR\n";
            send(c, response, strlen(response), 0);
        }

        close(c);
    }

    db_close(db);
    close(sock);
    return 0;
}