#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSIZE 1024
#define EOF_PACKET "EOFEOFEOFEOF*&^$#@!EOFEOFEOF!@#$^&*EOFEOFEOF"


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

    buf[0] = '!';
    buf[1] = '@';
    buf[2] = '#';
    strcpy(buf + 3, filename);

    //파일명 보내기 (앞에 !@#을 붙여서 파일임을 알림.)
    send(socket, buf, strlen(buf), 0);


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
                error_handling("select error() error");
                
            case 0:
                printf("Time over..! (resending filename..)\n");            
                send(socket, filename, strlen(filename), 0);
                break;
            default:
                read_bytes = recv(socket, buf, sizeof(buf), 0);
                printf("filename sent successfully!\n");
                loop = 0;
                break;
        }
    }

    while((send_bytes = fread(buf, sizeof(char), sizeof(buf) - 1, fp)) > 0) {
        send(socket, buf, send_bytes, 0);
        total_send_bytes += send_bytes;
        printf("%s sending (%d Bytes) ...   \r", filename, total_send_bytes);
        usleep(500);    //flow control을 해줌. (micro sec 단위)
    }

    memset(buf, sizeof(buf), 0);
    strcpy(buf, EOF_PACKET);

    //파일 내용이 모두 전송되면 eof packet 보내기.
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    send(socket, buf, sizeof(buf) - 1, 0);
    
    printf("\n%s sending done!                            \n", filename);
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