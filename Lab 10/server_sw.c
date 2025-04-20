#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/time.h>
#include<sys/types.h>

#define BUFLEN 512
#define PORT 8882
#define RETRANSMISSION_TIME 2
#define MOD 10

typedef struct packet {
    unsigned int size;
    unsigned int seq_num;
    bool is_last;
    bool is_data;
    char data[BUFLEN];
} PACKET;

void die(char *s) {
    perror(s);
    exit(1);
}

int main(void) {
    int client_sock;
    if ((client_sock=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        die("socket()");
    }
    struct sockaddr_in server_addr, client_addr;
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    if (bind(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr))==-1) {
        die("bind()");
    }
    int state=0;
    FILE *fp=fopen("output.txt", "w");
    if (fp==NULL) {
        die("fopen()");
    }
    PACKET rcv_pkt, ack_pkt;
    socklen_t client_addr_len=sizeof(client_addr);
    int recv_len;
    int r;
    while (1) {
        switch(state)
        {
            case 0:
            if ((recv_len=recvfrom(client_sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&client_addr, &client_addr_len))==-1) {
                die("recvfrom()");
            }
            r=rand();
            if (r%10==1) {
                printf("DROP DATA: Seq. No. %d of size %d bytes\n", rcv_pkt.seq_num, rcv_pkt.size);
                rcv_pkt.seq_num=-1;
            }
            if (rcv_pkt.is_data && !rcv_pkt.seq_num) {
                printf("RECEIVED DATA: Seq. No. %d of size %d bytes\n", rcv_pkt.seq_num, rcv_pkt.size);
                // printf("%s\n", rcv_pkt.data);
                fwrite(rcv_pkt.data, 1, strlen(rcv_pkt.data), fp);
                ack_pkt.seq_num=0;
                ack_pkt.is_data=0;
                ack_pkt.size=sizeof(ack_pkt);
                ack_pkt.is_last=rcv_pkt.is_last;
                if (sendto(client_sock, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&client_addr, client_addr_len)==-1) {
                    die("sendto()");
                }
                state=1;
                if (rcv_pkt.is_last) {
                    printf("TRANSFER COMPLETE\n");
                    close(client_sock);
                    fclose(fp);
                    return 0;
                }
            }
            break;

            case 1:
            if ((recv_len=recvfrom(client_sock, &rcv_pkt, sizeof(rcv_pkt), 0, (struct sockaddr *)&client_addr, &client_addr_len))==-1) {
                die("recvfrom()");
            }
            r=rand();
            if (r%10==1) {
                printf("DROP DATA: Seq. No. %d of size %d bytes\n", rcv_pkt.seq_num, rcv_pkt.size);
                rcv_pkt.seq_num=-1;
            }
            if (rcv_pkt.is_data && rcv_pkt.seq_num==1) {
                printf("RECEIVED DATA: Seq. No. %d of size %d bytes\n", rcv_pkt.seq_num, rcv_pkt.size);
                // printf("%s\n", rcv_pkt.data);
                fwrite(rcv_pkt.data, 1, strlen(rcv_pkt.data), fp);
                ack_pkt.seq_num=1;
                ack_pkt.is_data=0;
                ack_pkt.size=sizeof(ack_pkt);
                ack_pkt.is_last=rcv_pkt.is_last;
                if (sendto(client_sock, &ack_pkt, sizeof(ack_pkt), 0, (struct sockaddr *)&client_addr, client_addr_len)==-1) {
                    die("sendto()");
                }
                state=0;
                if (rcv_pkt.is_last) {
                    printf("TRANSFER COMPLETE\n");
                    close(client_sock);
                    fclose(fp);
                    return 0;
                }
            }
            break;
        }
    }
    close(client_sock);
    fclose(fp);
    return 0;
}