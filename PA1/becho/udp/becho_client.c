#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 30

void error_handling(const char * msg) {
    perror(msg);
    exit(1);
}

char MSG1[] = "Good ";
char MSG2[] = "Evening ";
char MSG3[] = "Everybody!";

int main(int argc, char ** argv) {
    if(argc != 3) {
        fprintf(stderr, "usage : %s <ip> <port>\n", argv[0]);
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

    sendto(clnt_sock, MSG1, strlen(MSG1), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    sendto(clnt_sock, MSG2, strlen(MSG2), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    sendto(clnt_sock, MSG3, strlen(MSG3), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    
    char recv_msg[BUFSIZE];
    int str_len;
    int i;
    for(i = 0; i < 3; i++) {
        from_addr_size = sizeof(from_addr);
        str_len = recvfrom(clnt_sock, recv_msg, sizeof(recv_msg) - 1, 0, (struct sockaddr *)&from_addr, &from_addr_size);
        if(str_len < 0) break;
        recv_msg[str_len] = 0;
        printf("recv msg from (%s/%d) : %s \n", inet_ntoa(from_addr.sin_addr), ntohs(from_addr.sin_port), recv_msg);
    }

    close(clnt_sock);
    return 0;
}