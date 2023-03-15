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
void *recv_thread(const int *arg);
void *accept_thread();
void test();

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
int status_thread_done = 0;

int main(int argc, char* argv[]){
    if(argc != 2)
        error_handling("usage: ./filename [port]");

    if((serv_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error_handling("socket error");

    memset(&serv_addr, 0 , sizeof(serv_addr));

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen error");

    pthread_create(&th_accept, NULL, accept_thread, NULL);

    while(cnt_clnt == 0){}

    printf("Hello this is test chat server!\n");
    printf("If you want to exit the chat, type [quit] on the screen.\n");

    while(1){ // send to all connected client
        if(status_exit)
            break;
        fgets(send_data, sizeof(send_data), stdin);
        if(strcmp(send_data, "test") == 10){
            test();
        }
        for(int i = 0 ; i < cnt_clnt ; i++){
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

void *recv_thread(const int *arg){
    const int index = *arg;
    status_thread_done = 1;
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
        if(status_exit)
                break;
        clnt_addr_size_list[cnt_clnt] = (int)sizeof(clnt_addr_list[cnt_clnt]);
        if((clnt_sock_list[cnt_clnt] = accept(serv_sock, (struct sockaddr *) &clnt_addr_list[cnt_clnt], &clnt_addr_size_list[cnt_clnt])) != -1){

            char msg[] = "Hello this is test chat server!\nIf you want to exit the chat, type [quit] on the screen.\n";
            write(clnt_sock_list[cnt_clnt], msg, sizeof(msg));

            printf("[new client enter], cnt_clnt: %d\n", cnt_clnt);

            pthread_create(&th_list[cnt_clnt], NULL, (void *(*)(void *)) &recv_thread, &cnt_clnt);
            while(!status_thread_done){}
            status_thread_done = 0;
            ++cnt_clnt;
        }
    }
    return NULL;
}

void test(){ // for check client information
    printf("\n---------\n[clnt_addr_list]\n");
    for(int i = 0 ; i < LIST_SIZE ; i++){
        printf("%s:%d, family: %d, len: %d\n", inet_ntoa(clnt_addr_list[i].sin_addr), clnt_addr_list[i].sin_port, clnt_addr_list[i].sin_family, clnt_addr_list[i].sin_len);
    }
    printf("---------\n[clnt_addr_size_list]\n");
    for(int i = 0 ; i < LIST_SIZE ; i++){
        printf("%d: %d\n", i, clnt_addr_size_list[i]);
    }
    printf("---------\n[clnt_sock_list]\n");
    for(int i = 0 ; i < LIST_SIZE ; i++){
        printf("%d: %d\n", i, clnt_sock_list[i]);
    }
    printf("---------\n\n");
}