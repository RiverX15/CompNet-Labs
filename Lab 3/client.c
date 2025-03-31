#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define BUFSIZE 32

int main() {
    int sock=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock<0) {
        printf("Error in opening a socket.\n");
        exit(0);
    }
    printf("Client socket created.\n");

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(12345);
    serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
    printf("Address assigned.\n");

    int c=connect(sock, (struct sockaddr*) &serverAddr, sizeof(serverAddr));
    // printf("%d\n", c);
    if (c<0) {
        printf("Error while establishing connection.\n");
        exit(0);
    }
    printf("Connection established.\n");

    printf("Enter message for server with max 32 characters:\n");
    char msg[BUFSIZE];
    gets(msg);
    int bytesSent=send(sock, msg, strlen(msg), 0);
    if (bytesSent!=strlen(msg)) {
        printf("Error while sending the message.\n");
        exit(0);
    }
    printf("Data sent.\n");

    char recvBuffer[BUFSIZE];
    int bytesRecvd=recv(sock, recvBuffer, BUFSIZE-1, 0);
    if (bytesRecvd<0) {
        printf("Error while receiving data from server.\n");
        exit(0);
    }
    recvBuffer[bytesRecvd]='\0';
    printf("%s\n", recvBuffer);

    close(sock);
    
    return 0;
}