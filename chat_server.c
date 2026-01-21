#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define PORT 9999 // 사용할 포트 번호

pid_t pid;

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    char message[BUF_SIZE];
    int str_len;

    // 1. 소켓 생성 (전화기 구입)
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
        error_handling("socket() error");

    // 주소 구조체 초기화
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 내 IP 자동 할당
    serv_addr.sin_port = htons(PORT);

    // 2. 주소 할당 (전화번호 부여)
    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    // 3. 연결 대기 (개통 완료)
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("Server is waiting on port %d...\n", PORT);

    // 4. 연결 수락 (수화기 듦)
    clnt_addr_size = sizeof(clnt_addr);
    // 여기서 클라이언트가 올 때까지 대기(Block)합니다.
    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

    if (clnt_sock == -1)
        error_handling("accept() error");
    else
        printf("Client Connected!\n");
    pid = fork(); // 연결된 뒤에 수신/발신 나누기

    // 5. 데이터 송수신

    if (pid == 0) // 수신(읽기)
    {
        while (1)
        {
            str_len = read(clnt_sock, message, BUF_SIZE - 1);

            if (str_len == 0 || str_len == -1)
            {
                break;
            }

            message[str_len] = 0;
            printf("Message from client: %s", message);
        }
        return 0;
    }

    else // 발신(쓰기)
    {
        while (1)
        {
            fputs("Input message(Q to quit): ", stdout);
            fgets(message, BUF_SIZE, stdin);

            if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
                break;

            // 서버로 전송
            write(clnt_sock, message, strlen(message));
        }
    }

    // 6. 종료
    close(clnt_sock);
    close(serv_sock);
    printf("Connection closed.\n");

    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}