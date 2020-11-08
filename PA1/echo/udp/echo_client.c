#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 0xff

void error_handling(const char * send_msg) {
    perror(send_msg);
    exit(1);
}

int main(int argc, char ** argv) {
    if(argc != 3) {
        fprintf(stderr, "usage : %s <ip addr> <port>\n", argv[0]);
        exit(1);
    }

    int clnt_sock;
    struct sockaddr_in serv_addr, from_addr;
    int from_addr_size;

    clnt_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(clnt_sock == -1)
        error_handling("socket() error");
    
    serv_addr.sin_family = AF_INET;
    if(inet_aton(argv[1], &serv_addr.sin_addr) == 0)
        error_handling("inet_aton() error");
    serv_addr.sin_port = htons(atoi(argv[2]));

    char send_msg[BUFSIZE];
    char recv_msg[BUFSIZE];
    int recv_len;
    while(1) {
        printf("Input message (q to quit) > ");
        fgets(send_msg, sizeof(send_msg) - 1, stdin);

        if(!strcmp(send_msg, "q\n"))
            break;

        sendto(clnt_sock, send_msg, strlen(send_msg), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

        from_addr_size = sizeof(from_addr);
        recv_len = recvfrom(clnt_sock, recv_msg, sizeof(recv_msg) - 1, 0, (struct sockaddr *) &from_addr, &from_addr_size);
        if(recv_len < 0)
            break;
            
        recv_msg[recv_len] = 0;
        printf("Massage from %s/%d : %s \n", inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port), recv_msg);
    }
    close(clnt_sock);
    return 0;
}