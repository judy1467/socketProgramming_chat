#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "pthread.h"

void error_handling(char *message);
void *recv_thread();
void *send_thread();

int client_fd;
char recv_data[102];
char send_data[102];
pthread_t th0;
pthread_t th1;


int main(int argc, char* argv[]) {
    struct sockaddr_in client_addr;

    // 소켓 생성&설정
    client_fd = socket(PF_INET,SOCK_STREAM,0);
    if(client_fd == -1){
        printf("socket error\n");
        exit(1);
    }

    // 인자를 변수에 할당
    char *ip = argv[1];
    in_port_t port = atoi(argv[2]);

    client_addr.sin_addr.s_addr = inet_addr(ip);
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(port);

    if(argc != 3){
        error_handling("usage: [ip] [port]");
    }

    if(connect(client_fd, (struct sockaddr *) &client_addr, sizeof(client_addr)) == -1){
        printf("connect error\n");
        exit(1);
    }

    pthread_t th0;
    pthread_t th1;

    pthread_create(&th0, NULL, recv_thread, NULL);
    pthread_create(&th1, NULL, send_thread, NULL);

    void *result;

    pthread_join(th0, &result);
    pthread_join(th1, &result);

    close(client_fd);


    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void start_thread(){
    pthread_create(&th0, NULL, recv_thread(), NULL);
    pthread_create(&th1, NULL, send_thread(), NULL);
}

void *recv_thread(){
    while(1){

        memset(recv_data, 0 , sizeof(send_data));
        if(recv(client_fd, recv_data, sizeof(recv_data), 0) != -1){
            recv_data[strlen(recv_data)] = '\0';
            printf("%s",recv_data);
        }

    }
    close(client_fd);
    return NULL;
}

void *send_thread(){
    while(1){
        printf("input message: ");
        fgets(send_data, sizeof (send_data), stdin);
        send(client_fd, send_data, sizeof(send_data), 0);
    }
    close(client_fd);
    return NULL;
}