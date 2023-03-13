#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include <arpa/inet.h>
#define LIST_SIZE 5

void error_handling(char *message);
void *recv_thread();
void *accept_thread();

struct sockaddr_in serv_addr;
struct sockaddr_in clnt_addr_list[LIST_SIZE];
socklen_t clnt_addr_size_list[LIST_SIZE];
int serv_sock;
int clnt_sock_list[LIST_SIZE];
char recv_data[1024];
char send_data[1024];
pthread_t th_accept;
pthread_t th_list[LIST_SIZE];

int status_exit = 0;
int cnt_clnt = 0;

int main(int argc, char* argv[]){

    if(argc != 2)
        error_handling("usage: ./filename [port]");

    if((serv_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error_handling("socket error");


    if(memset(&serv_addr, 0, sizeof(serv_addr)) == NULL)
        error_handling("memset Error!");

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen error");

    pthread_create(&th_accept, NULL, accept_thread, NULL);

    printf("Hello this is test chat server!\n");
    printf("If you want to exit the chat, type [quit] on the screen.\n");

    while(1){
        if(status_exit)
            break;
        fgets(send_data, sizeof(send_data), stdin);
        for(int i = 0 ; i < cnt_clnt +1; i++){
            send(clnt_sock_list[i], send_data, sizeof(send_data), 0);
        }
        if(strcmp(send_data, "quit") == 10)
            break;
    }

    for(int i = 0 ; i < cnt_clnt ; i++){
        close(clnt_sock_list[i]);
    }
    close(serv_sock);

    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void *recv_thread(){
    const int index = cnt_clnt;
    cnt_clnt += 1;
    while(1){
        if(recv(clnt_sock_list[index], recv_data, sizeof(recv_data), 0) != -1){
            printf("\nclient[%s:%d, index: %d]: %s\n", inet_ntoa(clnt_addr_list[index].sin_addr), clnt_addr_list[index].sin_port, index, recv_data);
            if(strcmp(recv_data, "quit") == 10){
                if(cnt_clnt > 1){
                    char msg[] = "quit\n";
                    for(int i = 0 ; i < cnt_clnt ; i++){
                        write(clnt_sock_list[i], msg, sizeof(msg));
                    }
                }
                printf("[disconnect!!]");
                status_exit = 1;
                break;
            }
        }
    }
    return NULL;
}

void *accept_thread(){
    while(1){
        if((clnt_sock_list[cnt_clnt] = accept(serv_sock, (struct sockaddr*)&clnt_addr_list[cnt_clnt], &clnt_addr_size_list[cnt_clnt])) != -1){
            if(status_exit)
                break;
            char msg[] = "Hello this is test chat server!\nIf you want to exit the chat, type [quit] on the screen.\n";
            write(clnt_sock_list[cnt_clnt], msg, sizeof(msg));
            pthread_create(&th_list[cnt_clnt], NULL, &recv_thread, NULL);
        }
    }
    return NULL;
}