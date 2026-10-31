#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8081
#define MAX_BUFFER 256

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    char* message = "STATUS";
    sendto(sock, message, strlen(message), 0, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));

    socklen_t len = sizeof(serv_addr);
    memset(buffer, 0, MAX_BUFFER);
    recvfrom(sock, buffer, MAX_BUFFER, 0, (struct sockaddr*)&serv_addr, &len);

    printf("UDP status query:\nSTATUS\n");
    printf("Response:\n%s\n", buffer);

    close(sock);
    return 0;
}