#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>

#define BUFLEN 512
#define PORT 5001
#define END_MARKER "EOF_MARKER"
#define END_MARKER_LEN 10

void die(char* s) {
    perror(s);
    exit(1);
}

int main(void) {
    struct sockaddr_in si_other;
    int socketFd, socketLen=sizeof(si_other);
    
    if ((socketFd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        die("socket()");
    }

    memset((char*)&si_other, 0, sizeof(si_other));
    si_other.sin_family=AF_INET;
    si_other.sin_port=htons(PORT);
    si_other.sin_addr.s_addr=inet_addr("127.0.0.1");

    FILE *fp;
    fp = fopen("destination_file.txt", "ab");
    if (fp==NULL) {
        die("fopen()");
    }
    fseek(fp, 0, SEEK_END);
    int offset = ftell(fp);
    printf("Enter 0 to get complete file, 1 to specify offset, 2 to calculate offset value from local file.\n");
    int choice;
    printf("Enter your choice: ");
    scanf("%d", &choice);
    char buf_command[BUFLEN];
    sprintf(buf_command, "%d", choice);
    if (sendto(socketFd, buf_command, strlen(buf_command), 0, (struct sockaddr *)&si_other, socketLen)==-1) {
        die("sendto()");
    }
    if (choice==1 || choice==2) {
        if (choice==1) {
            printf("Enter file offset: ");
            scanf("%d", &offset);
        }
        char buf_offset[BUFLEN];
        sprintf(buf_offset, "%d", offset);
        if (sendto(socketFd, buf_offset, strlen(buf_offset), 0, (struct sockaddr *)&si_other, socketLen)==-1) {
            die("sendto()");
        }
    }
    char buf_recv[BUFLEN];
    int bytesReceived, fileTransferComplete=0;
    while (!fileTransferComplete && ((bytesReceived=recvfrom(socketFd, buf_recv, BUFLEN, 0, (struct sockaddr *)&si_other, &socketLen))>0)) {
        if (bytesReceived==END_MARKER_LEN && !strncmp(END_MARKER, buf_recv, END_MARKER_LEN)) {
            printf("EOF received, file transfer completed.\n");
            fileTransferComplete=1;
        }
        else {
            printf("Bytes received: %d\n", bytesReceived);
            fwrite(buf_recv, 1, bytesReceived, fp);
            
        }
    }
    if (bytesReceived<0) {
        die("recvfrom()");
    }
    close(socketFd);
    fclose(fp);
    return 0;
}