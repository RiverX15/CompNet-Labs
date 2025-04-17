#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<sys/time.h>
#include<sys/types.h>

#define BUFLEN 512
#define PORT 8882
#define MOD 4

typedef struct {
    int seq_num;
} ACK_PKT;

typedef struct {
    int seq_num;
    char data[BUFLEN];
} DATA_PKT;

void die(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    int client_sock;
    if ((client_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        die("socket()");
    }
    struct sockaddr_in server_addr;
    socklen_t server_addr_len=sizeof(server_addr);
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    int state=0;
    DATA_PKT send_pkt;
    ACK_PKT rcv_ack;
    int r;
    fd_set read_fds;
    struct timeval timeout;
    int ret;
    while (1) {
        switch (state)
        {
            case 0:
            printf("Enter message 0: ");
            fgets(send_pkt.data, BUFLEN, stdin);
            send_pkt.seq_num=0;
            if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                die("sendto()");
            }
            state=1;
            break;

            case 1:
            FD_ZERO(&read_fds);
            FD_SET(client_sock, &read_fds);
            timeout.tv_sec=0;
            timeout.tv_usec=100000;
            ret=select(client_sock+1, &read_fds, NULL, NULL, &timeout);
            if (ret<0) {
                die("select()");
            }
            if (!ret) {
                printf("Timeout occured after 100 ms.\n");
                if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                    die("sendto()");
                }
                // state=1;
                break;
            }
            if (FD_ISSET(client_sock, &read_fds)) {
                if (recvfrom(client_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&server_addr, &server_addr_len)==-1) {
                    die("recvfrom()");
                }
                // // This code segment simulates ACK packet drops at sender (client).
                // // Would go into infinite loop because ACK packet retransmitting not implemented at receiver (server).
                // r=rand()%MOD;
                // printf("r: %d\n", r);
                // if (r==1) {
                //     rcv_ack.seq_num=-1;
                // }
                if (!rcv_ack.seq_num) {
                    printf("Received ACK seq_num %d\n", rcv_ack.seq_num);
                    state=2;
                }
            }
            break;

            case 2:
            printf("Enter message 1: ");
            fgets(send_pkt.data, BUFLEN, stdin);
            send_pkt.seq_num=1;
            if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                die("sendto()");
            }
            state=3;
            break;

            case 3:
            FD_ZERO(&read_fds);
            FD_SET(client_sock, &read_fds);
            timeout.tv_sec=0;
            timeout.tv_usec=100000;
            ret=select(client_sock+1, &read_fds, NULL, NULL, &timeout);
            if (ret<0) {
                die("select()");
            }
            if (!ret) {
                printf("Timeout occured after 100 ms.\n");
                if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                    die("sendto()");
                }
                // state=3;
                break;
            }
            if (FD_ISSET(client_sock, &read_fds)) {
                if (recvfrom(client_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&server_addr, &server_addr_len)==-1) {
                    die("recvfrom()");
                }
                // r=rand()%MOD;
                // printf("r: %d\n", r);
                // if (r==1) {
                //     rcv_ack.seq_num=-1;
                // }
                if (rcv_ack.seq_num==1) {
                    printf("Received ACK seq_num %d\n", rcv_ack.seq_num);
                    state=0;
                }
            }
            break;
        }
    }
    close(client_sock);
    return 0;
}