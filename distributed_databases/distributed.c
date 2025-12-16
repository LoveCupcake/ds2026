#include "distributed.h"
#include "db.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <stdio.h>

Peer peers[MAX_PEERS];
int peer_count = 0;

static void send_replica(Peer *p, const char *key, const char *value) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return;
    
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons(p->port);
    inet_pton(AF_INET, p->ip, &a.sin_addr);

    // Set timeout
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    if (connect(s, (struct sockaddr*)&a, sizeof(a)) == 0) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "REPL %s %s\n", key, value);
        send(s, buf, strlen(buf), 0);
        printf("[REPL] Sent to %s:%d - %s=%s\n", p->ip, p->port, key, value);
    } else {
        printf("[REPL] Failed to connect to %s:%d\n", p->ip, p->port);
    }
    close(s);
}

void dist_put(const char *key, const char *value) {
    // Store locally first
    db_put(global_db, key, value);
    
    printf("[DIST_PUT] Stored locally: %s=%s\n", key, value);

    // Replicate to all peers
    for (int i = 0; i < peer_count; i++) {
        send_replica(&peers[i], key, value);
    }
}