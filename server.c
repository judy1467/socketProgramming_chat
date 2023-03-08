#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "netinet/in.h"
#include "sys/socket.h"

void error_handling(char *message);
void *recv_thread();

// sockaddr_in 구조체는 sa_family(주소체계)가 AF_INET(IPv4)인 경우에 사용
struct sockaddr_in serv_addr, clnt_addr;

// socklen_t 는 소켓 관련 매개 변수에 사용됨
socklen_t clnt_addr_size;

int serv_sock;
int clnt_sock;
char recv_data[1024];
char send_data[1024];
pthread_t th0;

int status_exit = 0;

int main(int argc, char* argv[]){

    if(argc != 2)
        error_handling("usage: ./filename [port]");

    if((serv_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1)
        error_handling("socket error");


    if(memset(&serv_addr, 0, sizeof(serv_addr)) == NULL)
        error_handling("memset Error!");

    // sin_family는 addr, port와는 달리 네트워크를 통해 전송하는 변수가 아니라 커널에 의해서만 사용되는 변수이기 때문에 Host-Byte-Order순서가 맞다.
    serv_addr.sin_family=AF_INET;

    // htonl 은 32비트의 Host-Byte-Order(Little Endian)로 부터 TCP/IP에서 사용되는 Network-Byte-Order(Big Endian)로 변환하는 함수이다.
    // ntohl 은 htonl의 반대개념 (Network-Byte-Order -> Host-Byte-Order)
    // INADDR_ANY: 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미.
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind error");

    // 2번째 인자는 클라이언트를 대기시킬 수 있는 큐의 크기
    if(listen(serv_sock, 5) == -1)
        error_handling("listen error");

    clnt_addr_size = sizeof(clnt_addr);

    // accept의 반환값은 실패 시 -1, 성공 시 새로운 '디스크립터 번호'이다.
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if(clnt_sock == -1)
        error_handling("accept error");

    char msg[] = "Hello this is test server!\n[message's max size is 100bytes]\n";
    write(clnt_sock, msg, sizeof(msg));
    printf("Hello this is test server!\n[message's max size is 100bytes]\n");

    // recv쓰레드 생성
    pthread_create(&th0, NULL, recv_thread, NULL);

    while(1){
        if(status_exit)
            break;
        fgets(send_data, sizeof(send_data), stdin);
        send(clnt_sock, send_data, sizeof(send_data), 0);
    }

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
        if(recv(clnt_sock, recv_data, sizeof(recv_data), 0) == -1){
            printf("disconnect!!\n");
            return (int*)0;
        }
        else{
            if(strcmp(recv_data, "quit") == 10){
                break;
            }
            printf("%s",recv_data);
        }
    }
    status_exit = 1;
    close(clnt_sock);
    close(serv_sock);
    return NULL;
}
