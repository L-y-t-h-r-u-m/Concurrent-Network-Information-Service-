#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define MAX_BUFFER 256

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    char buffer[MAX_BUFFER];
    char domain[100], type[20];

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to Lookup Server. Type EXIT to quit.\n");

    while (1) {
        printf("Enter domain: ");
        fgets(domain, sizeof(domain), stdin);
        domain[strcspn(domain, "\n")] = 0; // Remove newline

        if (strcmp(domain, "EXIT") == 0) {
            send(sock, "EXIT", 4, 0);
            break;
        }

        printf("Enter type: ");
        fgets(type, sizeof(type), stdin);
        type[strcspn(type, "\n")] = 0;

        // Format and send query
        sprintf(buffer, "%s %s", domain, type);
        send(sock, buffer, strlen(buffer), 0);

        // Receive response
        memset(buffer, 0, MAX_BUFFER);
        int bytes = recv(sock, buffer, MAX_BUFFER - 1, 0);
        if (bytes <= 0) {
            printf("Server closed connection.\n");
            break;
        }

        printf("Server response:\n%s\n\n", buffer);
    }

    close(sock);
    return 0;
}