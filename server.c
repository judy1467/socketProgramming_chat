#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "netinet/in.h"
#include "sys/socket.h"
#include <arpa/inet.h>
#define LIST_SIZE 5

void send_to_client();
void error_handling(char *message);
void *recv_thread(const int *arg);
void *accept_thread();
void test();
void convert(int index);
void eliminate_char(char* str, char c);

struct sockaddr_in server_addr;
struct sockaddr_in client_addr_list[LIST_SIZE];
socklen_t client_addr_size_list[LIST_SIZE];
int server_sock;
int client_sock_list[LIST_SIZE];
char recv_data[1024];
char send_data[1024];
char convert_recv_data[1024];
pthread_t th_accept;
pthread_t th_list[LIST_SIZE];
char client_name[LIST_SIZE][1024];

int status_exit = 0;
int cnt_client = 0;
int status_thread_done = 0;

int main(int argc, char* argv[]){
    if(argc != 2)
        error_handling("usage: ./filename [port]");

    if((server_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error_handling("socket error");

    memset(&server_addr, 0 , sizeof(server_addr));

    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_port=htons(atoi(argv[1]));

    if(bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
        error_handling("bind error");

    if(listen(server_sock, 5) == -1)
        error_handling("listen error");

    printf("Hello this is test chat server!\n");
    printf("If you want to exit the chat, type [quit] on the screen.\n");
    printf("waiting client...\n");

    pthread_create(&th_accept, NULL, accept_thread, NULL);

    while(cnt_client == 0){} // wait for connection with client

    send_to_client();

    for(int i = 0 ; i < LIST_SIZE ; i++){
        pthread_join(th_list[i], (void**)NULL);
    }
    close(server_sock);

    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void send_to_client(){
    while(!status_exit){ // send to all connected client
        if(cnt_client == 0){
            break;
        }
        strcat(send_data, "server: ");
        fgets(send_data+8, sizeof(send_data)-8, stdin);
        if(strcmp(send_data, "server: test") == 10){
            test();
            continue;
        }
        else if(strcmp(send_data, "server: quit") == 10){
            for(int i = 0 ; i < LIST_SIZE ; i++)
                send(client_sock_list[i], send_data, sizeof(send_data), 0);

            pthread_cancel(th_accept);

            for(int i = 0 ; i < LIST_SIZE ; i++){
                pthread_cancel(th_list[i]);
            }
            break;
        }
        else if(strcmp(send_data, "server: ") == 10){
            continue;
        }
        for(int i = 0 ; i < LIST_SIZE ; i++)
            send(client_sock_list[i], send_data, sizeof(send_data), 0);
        memset(send_data, 0, sizeof(send_data));
    }
}

void *recv_thread(const int *arg){
    int first_data = 0;
    const int index = *arg;
    status_thread_done = 1;
    while(1){
        if(recv(client_sock_list[index], recv_data, sizeof(recv_data), 0) != -1){
            if(!first_data){
                memcpy(client_name[index], recv_data, sizeof(recv_data));
                eliminate_char(client_name[index], '\n');
                first_data = 1;
                continue;
            }

            if(strcmp(recv_data, "quit") == 10){
                close(client_sock_list[index]);
                printf("[%s] is out of this server.\n", client_name[index]);
                cnt_client--;
                break;
            }

            printf("\n%s[%s:%d, index: %d]: %s\n", client_name[index], inet_ntoa(client_addr_list[index].sin_addr), client_addr_list[index].sin_port, index, recv_data);

            convert(index);
            for(int i = 0 ; i < LIST_SIZE ; i++){
                if(i != index)
                    send(client_sock_list[i], convert_recv_data, sizeof(convert_recv_data), 0);
            }

            memset(convert_recv_data, 0, sizeof(convert_recv_data));
            memset(recv_data, 0, sizeof(recv_data));
        }
    }
    pthread_cancel(th_list[index]);
    return NULL;
}

void *accept_thread(){
    while(!status_exit){
        client_addr_size_list[cnt_client] = (int)sizeof(client_addr_list[cnt_client]);
        if((client_sock_list[cnt_client] = accept(server_sock, (struct sockaddr *) &client_addr_list[cnt_client], &client_addr_size_list[cnt_client])) != -1){

            char msg[] = "Hello! This is test chat server!\nIf you want to exit the chat, type [quit] on the screen.\n";
            write(client_sock_list[cnt_client], msg, sizeof(msg));

            printf("[new client enter], cnt_client: %d\n", cnt_client+1);

            pthread_create(&th_list[cnt_client], NULL, (void *(*)(void *)) &recv_thread, &cnt_client);
            while(!status_thread_done){}
            status_thread_done = 0;
            ++cnt_client;
        }
    }
    pthread_cancel(th_accept);
    return NULL;
}

void test(){ // for check client information
    printf("\n---------\n[client_addr_list]\n");
    for(int i = 0 ; i < LIST_SIZE ; i++){
        printf("%s:%d, family: %d, len: %d\n", inet_ntoa(client_addr_list[i].sin_addr), client_addr_list[i].sin_port, client_addr_list[i].sin_family, client_addr_list[i].sin_len);
    }
    printf("---------\n[client_addr_size_list]\n");
    for(int i = 0 ; i < LIST_SIZE ; i++){
        printf("%d: %d\n", i, client_addr_size_list[i]);
    }
    printf("---------\n[client_sock_list]\n");
    for(int i = 0 ; i < LIST_SIZE ; i++){
        printf("%d: %d\n", i, client_sock_list[i]);
    }
    printf("---------\n\n");
}

void convert(int index){
    strcat(convert_recv_data, client_name[index]);
    strcat(convert_recv_data, "[");
    strcat(convert_recv_data, inet_ntoa(client_addr_list[index].sin_addr));
    strcat(convert_recv_data, ":");

    char temp[6];
    sprintf(temp, "%d", client_addr_list[index].sin_port);

    strcat(convert_recv_data, temp);
    strcat(convert_recv_data, "]: ");
    strcat(convert_recv_data, recv_data);
}

void eliminate_char(char* str, char c){
    for(; *str != '\0' ; str++){
        if(*str == c){
            strcpy(str, str+1);
            str--;
        }
    }
}