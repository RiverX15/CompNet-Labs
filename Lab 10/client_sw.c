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
    struct sockaddr_in server_addr;
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    socklen_t server_addr_len=sizeof(server_addr);
    int state=0;
    FILE *fp=fopen("input.txt", "r");
    if (fp==NULL) {
        die("fopen()");
    }
    PACKET send_pkt, rcv_ack;
    struct timeval timeout;
    int ret;
    fd_set read_fds;
    while (1) {
        switch(state)
        {
            case 0:
            memset(send_pkt.data, 0, BUFLEN);
            if (fgets(send_pkt.data, BUFLEN, fp)==NULL) {
                send_pkt.is_last=1;
            }
            else {
                send_pkt.is_last=0;
            }
            send_pkt.is_data=1;
            // send_pkt.is_last|=(bool)feof(fp);
            send_pkt.seq_num=0;
            send_pkt.size=sizeof(send_pkt);
            if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                die("sendto()");
            }
            printf("SENT DATA: Seq. No. %d of size %d bytes\n", send_pkt.seq_num, send_pkt.size);
            // printf("data: %s\nis_last: %d\n", send_pkt.data, send_pkt.is_last);
            state=1;
            break;

            case 1:
            FD_ZERO(&read_fds);
            FD_SET(client_sock, &read_fds);
            timeout.tv_sec=RETRANSMISSION_TIME;
            timeout.tv_usec=0;
            ret=select(client_sock+1, &read_fds, NULL, NULL, &timeout);
            if (ret<0) {
                die("select()");
            }
            if (!ret) {
                if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                    die("sendto()");
                }
                printf("RESENT DATA: Seq. No. %d of size %d bytes\n", send_pkt.seq_num, send_pkt.size);
                break;
            }
            if (FD_ISSET(client_sock, &read_fds)) {
                if (recvfrom(client_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&server_addr, &server_addr_len)==-1) {
                    die("recvfrom()");
                }
                if (!rcv_ack.is_data && !rcv_ack.seq_num) {
                    printf("RECEIVED ACK: for packet with seq. no. %d\n", rcv_ack.seq_num);
                    state=2;
                    if (rcv_ack.is_last) {
                        printf("TRANSFER COMPLETE\n");
                        close(client_sock);
                        fclose(fp);
                        return 0;
                    }
                }
            }
            break;

            case 2:
            memset(send_pkt.data, 0, BUFLEN);
            if (fgets(send_pkt.data, BUFLEN, fp)==NULL) {
                send_pkt.is_last=1;
            }
            else {
                send_pkt.is_last=0;
            }
            send_pkt.is_data=1;
            // send_pkt.is_last|=(bool)feof(fp);
            send_pkt.seq_num=1;
            send_pkt.size=sizeof(send_pkt);
            if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                die("sendto()");
            }
            printf("SENT DATA: Seq. No. %d of size %d bytes\n", send_pkt.seq_num, send_pkt.size);
            // printf("data: %s\nis_last: %d\n", send_pkt.data, send_pkt.is_last);
            state=3;
            break;

            case 3:
            FD_ZERO(&read_fds);
            FD_SET(client_sock, &read_fds);
            timeout.tv_sec=RETRANSMISSION_TIME;
            timeout.tv_usec=0;
            ret=select(client_sock+1, &read_fds, NULL, NULL, &timeout);
            if (ret<0) {
                die("select()");
            }
            if (!ret) {
                if (sendto(client_sock, &send_pkt, sizeof(send_pkt), 0, (struct sockaddr *)&server_addr, server_addr_len)==-1) {
                    die("sendto()");
                }
                printf("RESENT DATA: Seq. No. %d of size %d bytes\n", send_pkt.seq_num, send_pkt.size);
                break;
            }
            if (FD_ISSET(client_sock, &read_fds)) {
                if (recvfrom(client_sock, &rcv_ack, sizeof(rcv_ack), 0, (struct sockaddr *)&server_addr, &server_addr_len)==-1) {
                    die("recvfrom()");
                }
                if (!rcv_ack.is_data && rcv_ack.seq_num==1) {
                    printf("RECEIVED ACK: for packet with seq. no. %d\n", rcv_ack.seq_num);
                    state=0;
                    if (rcv_ack.is_last) {
                        printf("TRANSFER COMPLETE\n");
                        close(client_sock);
                        fclose(fp);
                        return 0;
                    }
                }
            }
            break;
        }
    }
    close(client_sock);
    fclose(fp);
    return 0;
}