#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

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
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    int clnt_addr_size;

    listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(listen_sock == -1)
        error_handling("socket() error");

    ////////    here!

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error_handling("bind() error");

    ////////    here!

    
    if(listen(listen_sock, 5) == -1)
        error_handling("listen() error");

    ////////    here!

    
    printf("listening to %s port\n", argv[1]);

    clnt_addr_size = sizeof(clnt_addr);
    conn_sock = accept(listen_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
    if(conn_sock == -1)
        error_handling("accept() error");

    printf("client (%s/%d) connected!\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));


    char msg[0xFF];
    int str_len;
    while((str_len = recv(conn_sock, msg, sizeof(msg), 0)) != 0) {
        send(conn_sock, msg, str_len, 0);
        write(stdout, msg, str_len);
    }
    close(conn_sock);
    return 0;
}