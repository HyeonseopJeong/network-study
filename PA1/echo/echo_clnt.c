#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
    struct sockaddr_in serv_addr;

    clnt_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(clnt_sock == -1)
        error_handling("socket() error");
    
    serv_addr.sin_family = AF_INET;
    if(inet_aton(argv[1], &serv_addr.sin_addr) == 0)
        error_handling("inet_aton() error");
    serv_addr.sin_port = htons(atoi(argv[2]));

    if(connect(clnt_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");
    
    char send_msg[0xFF];
    char recv_msg[0xFF];
    int recv_len;
    while(1) {
        printf("Input message (q to quit) > ");
        fgets(send_msg, sizeof(send_msg), stdin);

        if(!strcmp(send_msg, "q"))
            break;

        send(clnt_sock, send_msg, strlen(send_msg), 0);
        if(recv_len = recv(clnt_sock, recv_msg, sizeof(recv_msg) - 1, 0) == 0)
            break;
        recv_msg[recv_len] = 0;
        printf("Massage from server : %s \n", recv_msg);
    }
    close(clnt_sock);
    return 0;
}