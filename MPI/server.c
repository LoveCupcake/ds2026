#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 4096

int main() {
    int listenfd, connfd;
    struct sockaddr_in servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(9000);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 5);

    printf("Server running on port 9000...\n");

    while (1) {
        connfd = accept(listenfd, NULL, NULL);

        char filename[256];
        recv(connfd, filename, sizeof(filename), 0);

        FILE *fp = fopen(filename, "rb");
        if (!fp) {
            send(connfd, "ERROR\n", 6, 0);
            close(connfd);
            continue;
        }

        fseek(fp, 0, SEEK_END);
        long filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char header[64];
        sprintf(header, "OK %ld\n", filesize);
        send(connfd, header, strlen(header), 0);

        char buffer[BUF_SIZE];
        long sent = 0;
        while (!feof(fp)) {
            int n = fread(buffer, 1, BUF_SIZE, fp);
            send(connfd, buffer, n, 0);
            sent += n;
        }
        fclose(fp);
        close(connfd);
    }
}
