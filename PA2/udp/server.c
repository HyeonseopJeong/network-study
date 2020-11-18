#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define EOF_PACKET "EOFEOFEOFEOF*&^$#@!EOFEOFEOF!@#$^&*EOFEOFEOF"

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

    usleep(100000);   //일부러 딜레이.. 쓰레기 값(이전 client의 잔여 packet)들이 다 도착하는걸 기다림. (0.1초)
    printf("wait for client...\n");

    //파일 명 받기 (이전 client의 packet들 무시하기.) -- '파일명 앞에 "!@#"를 붙이는 규칙으로 파일명임을 인식'
    while((read_bytes = recvfrom(socket, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &clnt_addr, &clnt_addr_size)) == 0
            || buf[0] != '!' || buf[1] != '@' || buf[2] != '#');


    buf[read_bytes] = 0;
    strcpy(filename, buf + 3);
    if(read_bytes < 0) {
        error_handling("filename recv() error");
    }
    filename[read_bytes] = 0;

    printf("filename : %s (from %s/%d)\n", filename, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));


    fp = fopen(filename, "wb");
    if(fp == NULL)
        error_handling("fopen() error");


    //파일명 잘 받았다는 ACK 보내기
    sendto(socket, buf, 0, 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
    
    //최대 2secs 기다리기
    int state;
    fd_set fd_status;
    struct timeval timeout;
    FD_ZERO(&fd_status);

    int loop = 1;

    while(loop) {
        FD_SET(socket, &fd_status);

        timeout.tv_sec = 2; //2초 타임아웃
        timeout.tv_usec = 0;

        state = select(socket + 1, &fd_status, 0, 0, &timeout);
        switch(state)
        {
            case -1:
                error_handling("select() error");
                
            case 0:
                printf("Time over..! (resending ACK)\n");            
                sendto(socket, buf, 0, 0, (struct sockaddr *)&clnt_addr, sizeof(clnt_addr));
                break;
            default:
                read_bytes = recvfrom(socket, buf, sizeof(buf), 0, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
                total_read_bytes += read_bytes;
                fwrite(buf, sizeof(char), read_bytes, fp);
                printf("start receiving file content!\n");
                loop = 0;
                break;
        }
    }

    while((read_bytes = recvfrom(socket, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &clnt_addr, &clnt_addr_size)) > 0) {
        if(strcmp(buf, EOF_PACKET) == 0)
            break;
        
        total_read_bytes += read_bytes;
        fwrite(buf, sizeof(char), read_bytes, fp);
        printf("%s receiving (%d Bytes) ...              \r", filename, total_read_bytes);
    }
    printf("\n%s receiving done!                            \n", filename);
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


    //struct timeval optVal = {3, 0}; 
    //int optLen = sizeof(optVal);
    //setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &optVal, optLen);

    while(1) {
        recv_file(serv_sock);    
    }

    close(serv_sock);
    return 0;

}