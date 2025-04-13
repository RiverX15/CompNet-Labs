#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8898
#define MAX_CONN 10
#define BUFFER_SIZE 1024

// Structure to pass client socket to thread function
typedef struct
{
    int client_sock;
    struct sockaddr_in client_addr;
} client_data_t;

// Thread function to handle each client
void *handle_client_threads(void *arg)
{
    client_data_t *data = (client_data_t *)arg;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    // Handle client interaction
    while ((bytes_read = read(data->client_sock, buffer, sizeof(buffer))) > 0)
    {
        write(data->client_sock, buffer, bytes_read);
    }
    if (!bytes_read)
    {
        printf("Client disconnected from %s:%d\n", inet_ntoa(data->client_addr.sin_addr), ntohs(data->client_addr.sin_port));
    }
    else if (bytes_read == -1)
    {
        perror("Error reading from socket");
    }
    close(data->client_sock);
    free(data); // Free allocated memory for client data
    pthread_exit(NULL);
}

void handle_client_fork(int client_sock)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    // Echo data received from client
    while ((bytes_read = read(client_sock, buffer, sizeof(buffer))) > 0)
    {
        write(client_sock, buffer, bytes_read);
    }
    if (!bytes_read)
    {
        printf("Client disconnected.\n");
    }
    else if (bytes_read == -1)
    {
        perror("Error reading from socket");
    }
    close(client_sock);
}

int main(int argc, char *argv[])
{
    int choice = 0;
    if (argc > 1)
    {
        if (!strcmp(argv[1], "fork"))
        {
            choice = 0;
        }
        else if (!strcmp(argv[1], "threads"))
        {
            choice = 1;
        }
    }
    printf("Server using %s...\n", (choice)?("threads"):("fork"));
    if (!choice)
    {
        int server_sock, client_sock;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        // Create socket
        if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Socket creation failed");
            exit(1);
        }
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_family = AF_INET;
        // Bind socket to address
        if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("Bind failed");
            close(server_sock);
            exit(1);
        }
        // Listen for incoming connections
        if (listen(server_sock, MAX_CONN) == -1)
        {
            perror("Listen failed");
            close(server_sock);
            exit(1);
        }
        printf("Server listening on port %d...\n", PORT);
        // Accept incoming connections and handle them concurrently
        while (1)
        {
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_sock == -1)
            {
                perror("Accept failed");
                continue;
            }
            printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            // Create a child process to handle the client
            if (!fork())
            {
                // Child process
                close(server_sock); // Child does not need the server socket
                handle_client_fork(client_sock);
                exit(0);
            }
            close(client_sock);
        }
        close(server_sock);
        return 0;
    }
    else
    {
        int server_sock, client_sock;
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        pthread_t thread_id;
        // Create server socket
        if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Socket creation failed");
            exit(1);
        }
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);
        // Bind socket to address
        if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("Bind failed");
            exit(1);
        }
        // Listen for incoming connections
        if (listen(server_sock, MAX_CONN) == -1)
        {
            perror("Listen failed");
            close(server_sock);
            exit(1);
        }
        printf("Server listening on port %d...\n", PORT);
        // Accept and handle client connections concurrently using threads
        while (1)
        {
            client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
            if (client_sock == -1)
            {
                perror("Accept failed");
                close(client_sock);
                continue;
            }
            printf("Client connected from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            // Allocate memory for client data and pass it to thread
            client_data_t *data = malloc(sizeof(client_data_t));
            data->client_addr = client_addr;
            data->client_sock = client_sock;
            // Create a new thread for client
            if (pthread_create(&thread_id, NULL, handle_client_threads, data) != 0)
            {
                perror("Thread creation failed");
                free(data);
                close(client_sock);
            }
            pthread_detach(thread_id);
        }
        close(server_sock);
        return 0;
    }
}