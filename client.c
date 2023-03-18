#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "sys/socket.h"
#include "pthread.h"

void send_to_server();
void error_handling(char *message);
void *recv_thread();

struct sockaddr_in server_addr;
socklen_t server_addr_size;
int client_sock;
char recv_data[1024];
char send_data[1024];
pthread_t th0;

int status_exit = 0;

int main(int argc, char* argv[]) {

    if(argc != 3)
        error_handling("usage: ./filename [ip] [port]");

    // 소켓 생성&설정
    client_sock = socket(PF_INET,SOCK_STREAM,0);
    if(client_sock == -1)
        error_handling("socket error");

    // 인자를 변수에 할당
    char *ip = argv[1];
    in_port_t port = atoi(argv[2]);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    server_addr_size = sizeof(server_addr);

    if(connect(client_sock, (struct sockaddr *) &server_addr, server_addr_size) == -1)
        error_handling("connect error");

    printf("Enter your name: ");
    fgets(send_data, sizeof(send_data), stdin);
    send(client_sock, send_data, sizeof(send_data), 0);
    memset(send_data, 0, sizeof(send_data));

    pthread_create(&th0, NULL, recv_thread, NULL);

    send_to_server();

    pthread_cancel(th0);
    pthread_join(th0, (void **)NULL);
    close(client_sock);

    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void send_to_server(){
    while(1){
        if(status_exit)
            break;
        fgets(send_data, sizeof(send_data), stdin);
        if(strcmp(send_data, "") == 10){
            continue;
        }
        printf("\n");
        send(client_sock, send_data, sizeof(send_data), 0);
        if(strcmp(send_data, "quit") == 10)
            break;
        memset(send_data, 0, sizeof(send_data));
    }
}

void *recv_thread(){
    while(1){
        if(recv(client_sock, recv_data, sizeof(recv_data), 0) == -1){
            printf("[disconnect!!]\n");
            return NULL;
        }
        else{
            if(strcmp(recv_data, "server: quit") == 10){
                printf("[disconnect!!]\n");
                break;
            }
            printf("%s\n", recv_data);
            memset(recv_data, 0, sizeof(recv_data));
        }
    }
    status_exit = 1;
    return NULL;
}