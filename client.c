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

int clnt_sock;
char recv_data[100];
char send_data[100];
pthread_t th0;
pthread_t th1;

struct sockaddr_in serv_addr;

int tmp;

int main(int argc, char* argv[]) {
    if(argc != 3){
        error_handling("usage: [ip] [port]");
    }

    // 소켓 생성&설정
    clnt_sock = socket(PF_INET,SOCK_STREAM,0);
    if(clnt_sock == -1){
        printf("socket error\n");
        exit(1);
    }

    // 인자를 변수에 할당
    char *ip = argv[1];
    in_port_t port = atoi(argv[2]);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if(connect(clnt_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1){
        printf("connect error\n");
        exit(1);
    }

    pthread_create(&th0, NULL, recv_thread, NULL);
    //pthread_create(&th1, NULL, send_thread, NULL);

    void *result;

    pthread_join(th0, &result);
    //pthread_join(th1, &result);

    while(1){
        memset(send_data, 0 , sizeof(send_data));
        fgets(send_data, sizeof (send_data), stdin);
        if(strcmp(send_data, "quit") == 0){
            break;
        }
        send(clnt_sock, send_data, sizeof(send_data), 0);
        while(getchar() != '\n');
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
        memset(recv_data, 0 , sizeof(recv_data));
        if((tmp=recv(clnt_sock, recv_data, sizeof(recv_data), 0)) == -1){
            printf("disconnect!!\n");
            return (int*)0;
        }
        else if(tmp > 0){
            if(strcmp(recv_data, "quit") == 0){
                break;
            }
            recv_data[tmp] = '\0';
            printf("%s",recv_data);
        }
    }
    close(clnt_sock);
    return NULL;
}

void *send_thread(){
    while(1){
        memset(send_data, 0 , sizeof(send_data));
        fgets(send_data, sizeof (send_data), stdin);
        if(strcmp(send_data, "quit") == 0){
            break;
        }
        send(clnt_sock, send_data, sizeof(send_data), 0);
        while(getchar() != '\n');
    }
    close(clnt_sock);
    return NULL;
}