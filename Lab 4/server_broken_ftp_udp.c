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
    struct sockaddr_in si_me, si_other;
    int socketFd, socketLen=sizeof(si_other);
    if ((socketFd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        die("socket()");
    }
    memset((char *)&si_me, 0, sizeof(si_me));
    si_me.sin_family=AF_INET;
    si_me.sin_port=htons(PORT);
    si_me.sin_addr.s_addr=htonl(INADDR_ANY);
    if (bind(socketFd, (struct sockaddr *)&si_me, sizeof(si_me))==-1) {
        die("bind()");
    }
    char buf[BUFLEN];
    int bytesReceived;
    printf("Waiting for client to send the command\n");
    if ((bytesReceived=recvfrom(socketFd, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &socketLen))<0) {
        die("recvfrom()");
    }
    int choice, offset;
    sscanf(buf, "%d", &choice);
    if (!choice) offset=0;
    else {
        printf("Waiting for client to send the offset\n");
        if ((bytesReceived=recvfrom(socketFd, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &socketLen))<0) {
            die("recvfrom()");
        }
        sscanf(buf, "%d", &offset);
    }
    FILE *fp=fopen("source_file.txt", "rb");
    if (fp==NULL) {
        die("fopen()");
    }
    fseek(fp, offset, SEEK_SET);
    while (1) {
        char buf_send[BUFLEN];
        int bytesRead=fread(buf_send, 1, BUFLEN, fp);
        printf("Bytes read: %d\n", bytesRead);
        if (bytesRead>0) {
            printf("Sending\n");
            if (sendto(socketFd, buf_send, bytesRead, 0, (struct sockaddr *)&si_other, socketLen)==-1) {
                die("sendto()");
            }
        }
        if (bytesRead<BUFLEN) {
            if (feof(fp)) {
                printf("EOF\n");
            }
            if (ferror(fp)) {
                perror("Error reading file");
            }
            break;
        }
    }
    printf("Sending EOF\n");
    if (sendto(socketFd, END_MARKER, END_MARKER_LEN, 0, (struct sockaddr *)&si_other, socketLen)==-1) {
        die("sendto()");
    }
    close(socketFd);
    fclose(fp);
    return 0;
}