#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

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
    printf("Enter message for server: ");
    char msg[BUFFER_SIZE];
    gets(msg);
    ssize_t bytesSent;
    if ((bytesSent=send(client_sock, msg, strlen(msg), 0))!=strlen(msg)) {
        perror("Sending message failed");
        exit(1);
    }
    char recv_buffer[BUFFER_SIZE];
    ssize_t bytesRecvd;
    if ((bytesRecvd=recv(client_sock, recv_buffer, BUFFER_SIZE-1, 0))<0) {
        perror("Receiving reply failed");
        exit(1);
    }
    recv_buffer[bytesRecvd]='\0';
    printf("Reply from server: %s\n", recv_buffer);
    close(client_sock);
    return 0;
}