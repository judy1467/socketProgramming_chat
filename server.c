#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "netinet/in.h"
#include "sys/socket.h"

void error_handling(char *message);
void *recv_thread();
void *send_thread();

// sockaddr_in 구조체는 sa_family(주소체계)가 AF_INET(IPv4)인 경우에 사용
struct sockaddr_in serv_addr, clnt_addr;

// socklen_t 는 소켓 관련 매개 변수에 사용됨
socklen_t clnt_addr_size;

static int serv_sock;
static int clnt_sock;

char recv_data[100];
char send_data[100];

int main(int argc, char* argv[]){

    if(argc != 2){
        printf("usage: ./filename [port]\n");
        exit(1);
    }

    if((serv_sock=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error_handling("socket error");
        exit(1);
    }

    if(memset(&serv_addr, 0, sizeof(serv_addr)) == NULL){
        printf("memset Error!");
        exit(1);
    }

    // sin_family는 addr, port와는 달리 네트워크를 통해 전송하는 변수가 아니라 커널에 의해서만 사용되는 변수이기 때문에 Host-Byte-Order순서가 맞다.
    serv_addr.sin_family=AF_INET;

    // htonl 은 32비트의 Host-Byte-Order(Little Endian)로 부터 TCP/IP에서 사용되는 Network-Byte-Order(Big Endian)로 변환하는 함수이다.
    // ntohl 은 htonl의 반대개념 (Network-Byte-Order -> Host-Byte-Order)
    // INADDR_ANY: 자동으로 이 컴퓨터에 존재하는 랜카드 중 사용가능한 랜카드의 IP주소를 사용하라는 의미.
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
        error_handling("bind error");
    }

    // 2번째 인자는 클라이언트를 대기시킬 수 있는 큐의 크기
    if(listen(serv_sock, 5) == -1){
        error_handling("listen error");
    }

    clnt_addr_size = sizeof(clnt_addr);

    // accept의 반환값은 실패 시 -1, 성공 시 새로운 '디스크립터 번호'이다.
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    if(clnt_sock == -1){
        error_handling("accept error");
    }

    char msg[] = "Hello this is test server!\n";
    write(clnt_sock, msg, sizeof(msg));
    printf("[message's max size is 100bytes]\n");
    printf("check1\n");

    pthread_t th0;
    pthread_t th1;

    pthread_create(&th0, NULL, recv_thread, NULL);
    pthread_create(&th1, NULL, send_thread, NULL);

    void *result;

    pthread_join(th0, &result);
    pthread_join(th1, &result);

    close(clnt_sock);
    close(serv_sock);

    return 0;
}

void error_handling(char *message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void *recv_thread(){
    while(1){
        memset(recv_data, 0 , sizeof(send_data));
        if(recv(clnt_sock, recv_data, sizeof(recv_data), 0) != -1){
            recv_data[strlen(recv_data)] = '\0';
            printf("%s",recv_data);
        }

    }
    close(clnt_sock);
    return NULL;
}

void *send_thread(){
    while(1){
        printf("input message: ");
        fgets(send_data, sizeof(send_data), stdin);
        send(clnt_sock, send_data, sizeof(send_data), 0);
    }
    close(clnt_sock);
    return NULL;
}