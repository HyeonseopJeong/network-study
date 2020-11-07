#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 5

void error_handling(const char * msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char ** argv) {
    
    if(argc != 2) {
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int listen_sock, conn_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    int clnt_addr_size;

    listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error_handling("bind() error");
    
    if(listen(listen_sock, 5) == -1)
        error_handling("listen() error");

    sleep(5);

    char recv_msg[BUFSIZE];
    int str_len;
    int num = 0;

    while(1) {
        clnt_addr_size = sizeof(clnt_addr);
        conn_sock = accept(listen_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
        if(conn_sock == -1)
            error_handling("accept() error");
        
        printf("client (%s/%d) connected!\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

        num = 0;

        while(1) {
            sleep(1);
            str_len = recv(conn_sock, recv_msg, sizeof(recv_msg), 0);
            if(str_len <= 0) {
                printf("client (%s/%d) disconnected!\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
                break;
            }
            printf("Receive num : %d \n", num++);
            send(conn_sock, recv_msg, str_len, 0);
        }
        close(conn_sock);
    }
    
    return 0;
}