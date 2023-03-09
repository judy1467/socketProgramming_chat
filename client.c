#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "pthread.h"

void error_handling(char *message);
void *recv_thread();

struct sockaddr_in serv_addr;
socklen_t serv_addr_size;
int clnt_sock;
char recv_data[1024];
char send_data[1024];
pthread_t th0;

int status_exit = 0;

int main(int argc, char* argv[]) {

    if(argc != 3)
        error_handling("usage: ./filename [ip] [port]");

    // 소켓 생성&설정
    clnt_sock = socket(PF_INET,SOCK_STREAM,0);
    if(clnt_sock == -1)
        error_handling("socket error");

    // 인자를 변수에 할당
    char *ip = argv[1];
    in_port_t port = atoi(argv[2]);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    serv_addr_size = sizeof(serv_addr);

    if(connect(clnt_sock, (struct sockaddr *) &serv_addr, serv_addr_size) == -1)
        error_handling("connect error");

    pthread_create(&th0, NULL, recv_thread, NULL);

    while(1){
        if(status_exit)
            break;
        fgets(send_data, sizeof(send_data), stdin);
        send(clnt_sock, send_data, sizeof(send_data), 0);
        if(strcmp(send_data, "quit") == 10)
            break;
    }

    close(clnt_sock);

    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void *recv_thread(){
    while(1){
        if(recv(clnt_sock, recv_data, sizeof(recv_data), 0) == -1){
            printf("[disconnect!!]\n");
            return NULL;
        }
        else{
            if(strcmp(recv_data, "quit") == 10){
                printf("[disconnect!!]\n");
                break;
            }
            printf("\nserver[%s]: %s\n", inet_ntoa(serv_addr.sin_addr), recv_data);
        }
    }
    status_exit = 1;
    return NULL;
}