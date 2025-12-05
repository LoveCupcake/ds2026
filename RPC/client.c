#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 4096

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    char filename[] = "test.txt";
    send(sockfd, filename, strlen(filename) + 1, 0);

    char response[64];
    recv(sockfd, response, sizeof(response), 0);

    if (strncmp(response, "OK", 2) != 0) {
        printf("Server error: %s", response);
        close(sockfd);
        return 0;
    }

    long filesize = atol(response + 3);
    FILE *fp = fopen("received.txt", "wb");

    char buffer[BUF_SIZE];
    long received = 0;

    while (received < filesize) {
        int n = recv(sockfd, buffer, BUF_SIZE, 0);
        fwrite(buffer, 1, n, fp);
        received += n;
    }

    fclose(fp);
    close(sockfd);
    return 0;
}
