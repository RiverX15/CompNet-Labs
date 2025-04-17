#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>

#define BUFLEN 512
#define PORT 8882

typedef struct packet1 {
    int seq_num;
} ACK_PKT;

typedef struct packet2 {
    int seq_num;
    char data[BUFLEN];
} DATA_PKT;

void die(char* s) {
    perror(s);
    exit(1);
}

int main(void) {
    int client_sock;
    if ((client_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        die("socket()");
    }
    struct sockaddr_in si_other;
    socklen_t sock_len=sizeof(si_other);
    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_addr.s_addr=inet_addr("127.0.0.1");
    si_other.sin_family=AF_INET;
    si_other.sin_port=htons(PORT);
    int state=0;
    DATA_PKT send_pkt, rcv_ack;
    while (1) {
        switch (state)
        {
            case 0:
            printf("Enter message 0: ");
            fgets(send_pkt.data, BUFLEN, stdin);
            send_pkt.seq_num=0;
            if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sock_len)==-1) {
                die("sendto()");
            }
            state=1;
            break;

            case 1:
            if (recvfrom(client_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &sock_len)==-1) {
                die("recvfrom()");
            }
            if (!rcv_ack.seq_num) {
                printf("Received ACK seq_num %d\n", rcv_ack.seq_num);
                state=2;
                break;
            }

            case 2:
            printf("Enter message 1: ");
            fgets(send_pkt.data, BUFLEN, stdin);
            send_pkt.seq_num=1;
            if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&si_other, sock_len)==-1) {
                die("sendto()");
            }
            state=3;
            break;

            case 3:
            if (recvfrom(client_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&si_other, &sock_len)==-1) {
                die("recvfrom()");
            }
            if (rcv_ack.seq_num==1) {
                printf("Received ACK seq_num %d\n", rcv_ack.seq_num);
                state=0;
                break;
            }
        }
    }
    close(client_sock);
    return 0;
}