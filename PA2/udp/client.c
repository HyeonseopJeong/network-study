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
send_file(int socket, const char * filename) {
    FILE * fp;
    char buf[BUFSIZE];
    int total_send_bytes = 0, send_bytes = 0;
    int read_bytes = 0;

    struct sockaddr_in addr;
    int addr_size = sizeof(addr);

    if((fp = fopen(filename, "rb")) == NULL) 
        error_handling("File not Exist");

    //파일명 보내기
    send(socket, filename, strlen(filename), 0);

    read_bytes = recv(socket, buf, sizeof(buf), 0);
    buf[read_bytes] = 0;
    if(strcmp(filename, buf) != 0) {
        buf[0] = 'N'; buf[1] = 'O'; buf[2] = 0; 
        send(socket, buf, strlen(buf), 0);
        printf("file name lost!\n");
        return 1;    
    }
    
    buf[0] = 'O'; buf[1] = 'K'; buf[2] = 0; 
    send(socket, buf, strlen(buf), 0);

    while((send_bytes = fread(buf, sizeof(char), sizeof(buf), fp)) > 0) {
        send(socket, buf, send_bytes, 0);
        total_send_bytes += send_bytes;
        printf("%s sending (%d Bytes) ...              \r", filename, total_send_bytes);
    }

    //파일 내용이 모두 전송되면 0 크기의 datagram 보내기.
    send(socket, buf, 0, 0);
    
    printf("%s sending done!                            \n", filename);
    fclose(fp);
    return 0;
}


int main(int argc, char ** argv) {
    if(argc != 4) {
        fprintf(stderr, "usage : %s <server_ip_address> <server_port_number> <file_name>\n", argv[0]);
        exit(1);
    }

    int clnt_sock;
    struct sockaddr_in serv_addr, from_addr;
    int from_addr_size;

    clnt_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(clnt_sock == -1) 
        error_handling("socket() error");
    
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    if(inet_aton(argv[1], &serv_addr.sin_addr) == 0)
        error_handling("inet_aton() error");


    if(connect(clnt_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    send_file(clnt_sock, argv[3]);
    
    close(clnt_sock);
    return 0;
}