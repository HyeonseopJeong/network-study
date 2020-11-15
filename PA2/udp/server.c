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
    char y_or_n = 0;

    struct sockaddr_in clnt_addr;
    int clnt_addr_size = sizeof(clnt_addr);

    //파일 명 받기
    read_bytes = recvfrom(socket, filename, sizeof(filename), 0, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
    if(read_bytes <= 0) {
        error_handling("filename recv() error");
    }
    filename[read_bytes] = 0;

    printf("Receiving from %s/%d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
    
    //파일명 체크
    sendto(socket, filename, strlen(filename), 0, (struct sockaddr *) &clnt_addr, sizeof(clnt_addr));
    read_bytes = recvfrom(socket, buf, sizeof(buf), 0, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
    buf[read_bytes] = 0;
    if(strcmp(buf, "OK") != 0) {
        printf("file name lost!\n");
        return 1;
    }

    fp = fopen(filename, "wb");

    while((read_bytes = recvfrom(socket, buf, sizeof(buf), 0, (struct sockaddr *) &clnt_addr, &clnt_addr_size)) > 0) {
        total_read_bytes += read_bytes;
        fwrite(buf, sizeof(char), read_bytes, fp);
    }
    printf("%s receiving done (%d Bytes)\n", filename, total_read_bytes);
    fclose(fp);
    return 0;
}


int main(int argc, char ** argv) {
    
    if(argc != 2) {
        fprintf(stderr, "usage : %s <port>\n", argv[0]);
        exit(1);
    }

    int serv_sock;
    struct sockaddr_in serv_addr;

    serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if( bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error_handling("bind() error");

    char filename[BUFSIZE];

    while(1) {
        recv_file(serv_sock);    
    }

    close(serv_sock);
    return 0;

}