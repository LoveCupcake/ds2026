#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage:\n");
        printf("  PUT: %s <port> PUT <key> <value>\n", argv[0]);
        printf("  GET: %s <port> GET <key>\n", argv[0]);
        printf("\nExamples:\n");
        printf("  %s 8000 PUT user1 Name\n", argv[0]);
        printf("  %s 8000 GET user1\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    const char *cmd = argv[2];
    const char *key = argv[3];

    // Create socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }
    
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &a.sin_addr);

    // Connect to server
    if (connect(s, (struct sockaddr*)&a, sizeof(a)) < 0) {
        perror("connect");
        close(s);
        return 1;
    }

    char buf[1024];
    
    // Send command
    if (strcmp(cmd, "PUT") == 0) {
        if (argc < 5) {
            printf("PUT requires value: %s <port> PUT <key> <value>\n", argv[0]);
            close(s);
            return 1;
        }
        const char *value = argv[4];
        snprintf(buf, sizeof(buf), "PUT %s %s\n", key, value);
        printf("Sending: %s", buf);
        
    } else if (strcmp(cmd, "GET") == 0) {
        snprintf(buf, sizeof(buf), "GET %s\n", key);
        printf("Sending: %s", buf);
        
    } else {
        printf("Unknown command: %s\n", cmd);
        printf("Use PUT or GET\n");
        close(s);
        return 1;
    }

    // Send request
    if (send(s, buf, strlen(buf), 0) < 0) {
        perror("send");
        close(s);
        return 1;
    }

    // Receive response
    memset(buf, 0, sizeof(buf));
    ssize_t n = recv(s, buf, sizeof(buf) - 1, 0);
    
    if (n > 0) {
        buf[n] = '\0';
        printf("Response: %s", buf);
    } else {
        printf("No response from server\n");
    }

    close(s);
    return 0;
}