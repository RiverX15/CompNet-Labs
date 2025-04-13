#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

#define PORT 8898
#define BUFFER_SIZE 1024

int main() {
    int client_sock=socket(PF_INET, SOCK_STREAM, 0);
    if (client_sock<0) {
        perror("Socket creation failed");
        exit(1);
    }
    struct sockaddr_in server_addr;
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))<0) {
        perror("Connection failed");
        exit(1);
    }
    while (1) {
        char input[BUFFER_SIZE];
        printf("Enter command > ");
        gets(input);
        ssize_t bytes_sent;
        if ((bytes_sent=send(client_sock, input, strlen(input), 0))!=strlen(input)) {
            perror("Sending message failed");
            exit(1);
        }
        char recv_buffer[BUFFER_SIZE];
        ssize_t bytes_recvd;
        if ((bytes_recvd=recv(client_sock, recv_buffer, BUFFER_SIZE-1, 0))<0) {
            perror("Receiving reply failed");
            exit(1);
        }
        recv_buffer[bytes_recvd]='\0';
        printf("%s\n", recv_buffer);
        if (!strcmp(recv_buffer, "Goodbye")) {
            break;
        }
    }
    close(client_sock);
    return 0;
}