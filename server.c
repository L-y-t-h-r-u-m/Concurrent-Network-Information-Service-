#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define TCP_PORT 8080
#define UDP_PORT 8081
#define MAX_BUFFER 256

// DNS Record Structure
struct Record {
    char host[50];
    char type[10];
    char value[50];
};

// Predefined Database
struct Record dns_db[] = {
    {"www.example.com", "A", "192.168.1.10"},
    {"mail.example.com", "A", "192.168.1.20"},
    {"web.example.com", "CNAME", "www.example.com"},
    {"example.com", "MX", "mail.example.com"},
    {"example.com", "NS", "ns1.example.com"}
};
int db_size = 5;

// Shared state for UDP Status
int connected_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Lookup Function
char* lookup_dns(char* host, char* type) {
    for (int i = 0; i < db_size; i++) {
        if (strcmp(dns_db[i].host, host) == 0 && strcmp(dns_db[i].type, type) == 0) {
            return dns_db[i].value;
        }
    }
    return "Record not found";
}

// TCP Client Handler Thread
void* handle_tcp_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg); // Free the dynamically allocated memory for the socket descriptor
    char buffer[MAX_BUFFER];

    while (1) {
        memset(buffer, 0, MAX_BUFFER);
        int bytes_read = recv(client_socket, buffer, MAX_BUFFER - 1, 0);
        
        if (bytes_read <= 0 || strncmp(buffer, "EXIT", 4) == 0) {
            break; // Client disconnected or requested exit
        }

        // Parse query "domain type"
        char host[50] = {0}, type[10] = {0};
        if (sscanf(buffer, "%s %s", host, type) == 2) {
            char* result = lookup_dns(host, type);
            send(client_socket, result, strlen(result), 0);
        } else {
            char* err = "Invalid query format.";
            send(client_socket, err, strlen(err), 0);
        }
    }

    // Client disconnecting, decrement counter
    pthread_mutex_lock(&clients_mutex);
    connected_clients--;
    pthread_mutex_unlock(&clients_mutex);
    
    close(client_socket);
    pthread_exit(NULL);
}

// UDP Status Handler Thread
void* handle_udp_server(void* arg) {
    int udp_socket;
    struct sockaddr_in server_addr, client_addr;
    char buffer[MAX_BUFFER];
    socklen_t addr_len = sizeof(client_addr);

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT);

    bind(udp_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("UDP Status Server running on port %d\n", UDP_PORT);

    while (1) {
        memset(buffer, 0, MAX_BUFFER);
        recvfrom(udp_socket, buffer, MAX_BUFFER, 0, (struct sockaddr*)&client_addr, &addr_len);
        
        // Remove trailing newlines
        buffer[strcspn(buffer, "\r\n")] = 0;

        if (strcmp(buffer, "STATUS") == 0) {
            char response[MAX_BUFFER];
            pthread_mutex_lock(&clients_mutex);
            sprintf(response, "Server active. Connected clients: %d", connected_clients);
            pthread_mutex_unlock(&clients_mutex);
            
            sendto(udp_socket, response, strlen(response), 0, (struct sockaddr*)&client_addr, addr_len);
        }
    }
    return NULL;
}

int main() {
    int server_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    // Start UDP Server thread
    pthread_t udp_thread;
    pthread_create(&udp_thread, NULL, handle_udp_server, NULL);

    // Setup TCP Server
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(TCP_PORT);

    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 10);
    printf("TCP Lookup Server running on port %d\n", TCP_PORT);

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
        if (client_socket < 0) continue;

        // Log client connection details
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);
        printf("[+] New TCP Client Connected: IP %s, Port %d\n", client_ip, client_port);

        // Increment connected clients safely
        pthread_mutex_lock(&clients_mutex);
        connected_clients++;
        pthread_mutex_unlock(&clients_mutex);

        // Spawn thread for TCP client
        pthread_t client_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;
        pthread_create(&client_thread, NULL, handle_tcp_client, (void*)new_sock);
        pthread_detach(client_thread); // Auto-reap resources when thread exits
    }

    close(server_socket);
    return 0;
}