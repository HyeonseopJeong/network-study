#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

void error_handling(const char * msg) {
    perror(msg);
    exit(1);
}


int 
recv_file(int socket) {
    
    FILE * fp;
    char buf[BUFSIZE];
    char filename[BUFSIZE];
    int total_read_bytes = 0, read_bytes = 0;

    //파일 명 받기
    read_bytes = recv(socket, filename, sizeof(filename), 0);
    if(read_bytes <= 0) {
        error_handling("filename recv() error");
    }
    //filename[read_bytes] = 0;
    printf("filename (%d bytes) : %s\n", strlen(filename), filename);
    fp = fopen(filename, "wb");

    while((read_bytes = recv(socket, buf, sizeof(buf), 0)) > 0) {
        total_read_bytes += read_bytes;
        fwrite(buf, sizeof(char), read_bytes, fp);
        printf("%s receiving (%d Bytes) ...              \r", filename, total_read_bytes);
    }
    
    printf("%s receiving done!                            \n", filename);
    fclose(fp);
    return 0;
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

    char recv_msg[BUFSIZE];
    int str_len;
    int num = 0;

    while(1) {
        clnt_addr_size = sizeof(clnt_addr);
        conn_sock = accept(listen_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
        if(conn_sock == -1)
            error_handling("accept() error");
        
        printf("client (%s/%d) connected!\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        recv_file(conn_sock);
        
        close(conn_sock);
    }
    
    return 0;
}