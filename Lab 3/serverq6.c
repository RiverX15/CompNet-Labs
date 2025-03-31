#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>

#define MAXPENDING 5
#define BUFSIZE 32

int main() {
    int serverSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket<0) {
        printf("Error while server socket creation.\n");
        exit(0);
    }
    printf("Server socket created.\n");

    struct sockaddr_in serverAddress, clientAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family=AF_INET;
    serverAddress.sin_port=htons(12345);
    // serverAddress.sin_addr.s_addr=htonl(INADDR_ANY);
    serverAddress.sin_addr.s_addr=inet_addr("127.0.0.1");
    // printf("%d\n", serverAddress.sin_addr.s_addr);
    printf("Server address assigned.\n");

    int temp=bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
    if (temp<0) {
        printf("Error while binding.\n");
        exit(0);
    }
    printf("Binding successful.\n");

    int temp1=listen(serverSocket, MAXPENDING);
    if (temp1<0) {
        printf("Error in listen().\n");
        exit(0);
    }
    printf("Now listening.\n");

    char msg[BUFSIZE];
    int clientLength=sizeof(clientAddress);
    int clientSocket=accept(serverSocket, (struct sockaddr*) &clientAddress, &clientLength);
    if (clientLength<0) {
        printf("Error in client socket.\n");
        exit(0);
    }
    printf("Handling client %s\n", inet_ntoa(clientAddress.sin_addr));
    int temp2=recv(clientSocket, msg, BUFSIZE, 0);
    if (temp2<0) {
        printf("Problem in temp2.\n");
        exit(0);
    }
    float num=atof(msg);
    printf("Real number received from client: %f\n", num);
    int higherNum=num;
    ++higherNum;
    printf("Integer for client: %d\n", higherNum);
    // printf("Enter message for client:\n");
    // gets(msg);
    snprintf(msg, BUFSIZE, "%d", higherNum);
    // printf("%s\n", msg);
    int bytesSent=send(clientSocket, msg, strlen(msg), 0);
    if (bytesSent!=strlen(msg)) {
        printf("Error while sending message to client.\n");
        exit(0);
    }
    close(serverSocket);
    close(clientSocket);

    return 0;
}