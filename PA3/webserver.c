#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 4096


//헤더 필드
struct HTTPHeader{
    char *name;
    char *value;

    struct HTTPHeader *next;   // linked list로 여러 header 묶음.
};


struct HTTPRequest {
    char *method;
    char *path;
    int minor_version;
                 
    struct HTTPHeader *header;      

    char *body;                         
    int body_length;

    int error_check;
};


void error_handling(const char * msg) {
    perror(msg);
    exit(1);
}

void print_HTTPRequst(struct HTTPRequest * req) {
    if(req->error_check) {
        printf("This is error request!\n");
        return;
    }
    if(req->method) {
        printf("method : %s\n", req->method);
    }
    if(req->path) {
        printf("path : %s\n", req->path);
    }
    printf("minor_version : %d\n", req->minor_version);

    printf("body length : %d\n", req->body_length);
    if(req->body_length) {
        printf("-----body-----\n");
        printf("%s\n", req->body);
        printf("--------------\n");
    }


    printf("-- Headers -- \n");
    struct HTTPHeader * p = req->header;

    while(p) {
        printf("%s : %s\n", p->name, p->value);
        p = p->next;
    }

    puts(""); puts("");
    
}


void free_request(struct HTTPRequest * req) {
    if(req->method) {
        free(req->method);
    }
    if(req->path) {
        free(req->path);
    }
    

    struct HTTPHeader * cur = req->header;
    struct HTTPHeader * prev;
    while(!cur) {
        free(cur->name); free(cur->value);
        
        prev = cur;
        cur = cur->next;
        free(prev);
    }

    if(req->body) {
        free(req->body);
    }

    free(req);

}


char * get_header_value (struct HTTPRequest * req, char * header_name){
    struct HTTPHeader * p = req->header;

    while(p) {
        if(strcmp(p->name, header_name) == 0)
            return p->value;
        p = p->next;
    }
    return NULL;
}


struct HTTPRequest* parse_request_all(FILE *in){
    
    int i;

    char buf[BUF_SIZE];
    char * path;
    char * ptr;


    struct HTTPRequest *req;
    struct HTTPHeader *h;
    
    req = (struct HTTPRequest *)malloc(sizeof(struct HTTPRequest));
    if(!req)
        error_handling("malloc() error");

    
    //request line 파싱하기
    
    if(!fgets(buf, sizeof(buf), in))
        error_handling("fgets() error1");


    //method parsing
    ptr = strchr(buf, ' ');
    if(!ptr) {
        printf("method parsing error!\n");
        req->error_check = 1;
        return req;
    }
    *ptr = '\0';
    ptr += 1;


    //method 소문자로 변환
    for(i = 0; buf[i] != '\0'; i++) {
        if('A' <= buf[i]  && buf[i] <= 'Z')
            buf[i] = buf[i] - 'A' + 'a';
    }

    req->method = (char*)malloc(strlen(buf) + 1);
    if(!req->method)
        error_handling("malloc() error");

    strcpy(req->method, buf);

    if(strcmp(req->method, "get") != 0 && strcmp(req->method, "post") != 0) {
        printf("method parsing error2!\n");
        req->error_check = 1;
        return req;
    }


    //path parsing
    path = ptr;
    ptr = strchr(path, ' ');
    if(!ptr) {
        printf("path parsing error!\n");
        req->error_check = 1;
        return req;
    }

    *ptr = '\0';
    ptr += 1;

    req->path = (char*)malloc(strlen(path) + 1);
    strcpy(req->path, path);

    //minor version parsing
    if(strncmp(ptr, "HTTP/", 5) != 0) {
        printf("version parsing error!\n");
        req->error_check = 1;
        return req;
    }
    req->minor_version = atoi(ptr + 7);

    
    //request line parsing done.

    //printf("check!\n");
    


    //request headers 파싱하기.
    req->header = NULL;
    while (1) {
        
        if(!fgets(buf, sizeof(buf), in))
            error_handling("fgets() error2");
        
        if(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
            break;
        
        ptr = strchr(buf, ':');
        if(!ptr) {
            printf("headers parsing error!\n");
            req->error_check = 1;
            return req;
        }
        *ptr = '\0';
        ptr += 1;
        if(*ptr == ' ')
            ptr += 1;


        //header name 소문자로 변환
        for(i = 0; buf[i] != '\0'; i++) {
            if('A' <= buf[i]  && buf[i] <= 'Z')
               buf[i] = buf[i] - 'A' + 'a';
        }

        struct HTTPHeader * new_header = (struct HTTPHeader *)malloc(sizeof(struct HTTPHeader));
        new_header->name = (char *)malloc(strlen(buf) + 1);
        strcpy(new_header->name, buf);
        
        new_header->value = (char *)malloc(strlen(ptr) + 1);
        strcpy(new_header->value, ptr);

        new_header->next = req->header;
        req->header = new_header;
    }
    
    //printf("check!\n");

    //body length parsing
    ptr = get_header_value(req, "content-length");
    if(!ptr)
        req->body_length = 0;
    else
        req->body_length = atoi(ptr);
    
    req->body = NULL;

    if (req->body_length != 0) {
        
        if (req->body_length >= BUF_SIZE) {
            printf("body length is too long.\n");
            req->error_check = 1;
            return req;
        }
        
        //request body 메모리 할당
        req->body = (char*)malloc(req->body_length + 1);
        memset(req->body, 0, sizeof(req->body));

        //request body 내용 읽기.
        if (fread(req->body, 1,  req->body_length, in) < 1) {
            error_handling("fread() error");
        }
    } 
    

    return  req;
}


void response_to(struct HTTPRequest * req, FILE * out) {
    if(req->error_check) {
        //404
        fprintf(out, "HTTP/1.%d 404 Not Found\r\n", req->minor_version);
        fflush(out);
        printf("404 errorcheck\n");
        return;
    }

    if(!req->path) {
        //404
        fprintf(out, "HTTP/1.%d 404 Not Found\r\n", req->minor_version);
        fflush(out);
        printf("404 path null\n");
        return;
    }

    char file_path[BUF_SIZE] = ".";
    char buf[BUF_SIZE];

    
    strcat(file_path, req->path);

    if(strcmp(req->method, "get") == 0) {
         
        // path가 /(루트) 일 경우 index.html로 바꿔줌.
        if(strcmp(file_path, "./") == 0) {
            strcat(file_path, "index.html");
        }

        //index.html 혹은 query.html이 아닐경우 404
        if(strcmp(file_path, "./index.html") && strcmp(file_path, "./query.html")) {
            fprintf(out, "HTTP/1.%d 404 Not Found\r\n", req->minor_version);
            fflush(out);
            printf("404 no file\n");
            return;
        }

        FILE * fp = fopen(file_path, "r");
        //해당 파일이 있는지 확인
        if(!fp) {//없으면
            //404
            fprintf(out, "HTTP/1.%d 404 Not Found\r\n", req->minor_version);
            fflush(out);
            printf("404 no file\n");
            return;
        }



        // 응답 !!
        printf("response ready!!\n");

        fseek(fp, 0, SEEK_END); 
        int file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        
        //response status line
        fprintf(out, "HTTP/1.%d 200 OK\r\n", req->minor_version);
        fprintf(out, "Content-Length:%d\r\n", file_size);
        fprintf(out, "Content-Type:text/html\r\n");
        fprintf(out, "\r\n");
        
        while(feof(fp) == 0) {
            memset(buf, 0, sizeof(buf));
            fread(buf, sizeof(char), BUF_SIZE - 1, fp);
            fprintf(out, "%s", buf);
        }
        fflush(out);
        fclose(fp);
    }
    else if(strcmp(req->method, "post") == 0) { //post일 경우 path에 상관없이 그냥 body로 받은 data를 출력.
        
        char body[BUF_SIZE];
        memset(body, 0, sizeof(body));

        


        //보낼 body 담기.


        char * body_ptr = body;
        body_ptr += sprintf(body_ptr, "<!DOCTYPE html>\n");
        body_ptr += sprintf(body_ptr, "<html>\n");
        body_ptr += sprintf(body_ptr, "\t<body>\n");

       
        
        char * q;
        char * ptr = strtok(req->body, "&");
        while(ptr) {
            q = strchr(ptr, '=');
            *q = '\0';  
            q += 1;
            body_ptr += sprintf(body_ptr, "\t\t%s is %s..!\n", ptr, q);
            ptr = strtok(NULL, "&");
        }

        //fprintf(out, "here i go far away i'll keep running in this game.\n");
        body_ptr += sprintf(body_ptr, "\t</body>\n");
        body_ptr += sprintf(body_ptr, "</html>");


        printf("body is ...\n");
        printf("%s\n", body);

        //응답하기!

        //response status line
        fprintf(out, "HTTP/1.%d 200 OK\r\n", req->minor_version);
        fprintf(out, "Content-Length:%d\r\n", strlen(body));
        fprintf(out, "Content-Type:text/html\r\n");
        fprintf(out, "\r\n");
        fprintf(out, "%s", body);
        fflush(out);
    }
    else {
        //404
        fprintf(out, "HTTP/1.%d 404 Not Found\r\n", req->minor_version);
        fflush(out);
        printf("404 method is not get nor post\n");
    }

}




//HTTP 요청 처리 + 응답 처리
void service(FILE *in , FILE *out){
    struct HTTPRequest *req;
    
    req = parse_request_all(in);
    //printf("check!\n");
    print_HTTPRequst(req);

    response_to(req, out);
    
    free_request(req);
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


    while(1) {
        clnt_addr_size = sizeof(clnt_addr);
        conn_sock = accept(listen_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
        if(conn_sock == -1)
            error_handling("accept() error");
        
        printf("client (%s:%d) connected!\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));
        
        FILE * in = fdopen(conn_sock, "r");    //socket을 한 라인씩 읽기위해 고수준 FILE pointer로 변환.
        FILE * out = fdopen(conn_sock, "w");

        service(in, out);
        
        close(conn_sock);
    }
    
    return 0;
}