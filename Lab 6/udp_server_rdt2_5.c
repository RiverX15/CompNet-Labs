#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>

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

void die(char* s) {
    perror(s);
    exit(1);
}

int main(void) {
    int server_sock;
    if ((server_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        die("socket()");
    }
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len=sizeof(client_addr), recv_len;
    memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1) {
        die("bind()");
    }
    int state=0;
    DATA_PKT rcv_pkt;
    ACK_PKT ack_pkt;
    int r;
    while (1) {
        switch(state)
        {
            case 0:
            printf("Waiting for packet 0 from sender...\n");
            fflush(stdout);
            if ((recv_len=recvfrom(server_sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&client_addr, &client_addr_len))==-1) {
                die("recvfrom()");
            }
            r=rand()%MOD;
            // printf("r: %d\n", r);
            if (r==1) {
                rcv_pkt.seq_num=-1;
                printf("------------ Packet drop -----------------\n");
            }
            if (!rcv_pkt.seq_num) {
                printf("Packet received with seq_num %d and packet content: %s\n", rcv_pkt.seq_num, rcv_pkt.data);
                ack_pkt.seq_num=rcv_pkt.seq_num;
                if (sendto(server_sock, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&client_addr, client_addr_len)==-1) {
                    die("sendto()");
                }
                state=1;
            }
            break;

            case 1:
            printf("Waiting for packet 1 from sender...\n");
            fflush(stdout);
            if ((recv_len=recvfrom(server_sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&client_addr, &client_addr_len))==-1) {
                die("recvfrom()");
            }
            r=rand()%MOD;
            // printf("r: %d\n", r);
            if (r==1) {
                rcv_pkt.seq_num=-1;
                printf("------------ Packet drop -----------------\n");
            }
            if (rcv_pkt.seq_num==1) {
                printf("Packet received with seq_num %d and packet content: %s\n", rcv_pkt.seq_num, rcv_pkt.data);
                ack_pkt.seq_num=rcv_pkt.seq_num;
                if (sendto(server_sock, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&client_addr, client_addr_len)==-1) {
                    die("sendto()");
                }
                state=0;
            }
            break;
        }
    }
    close(server_sock);
    return 0;
}