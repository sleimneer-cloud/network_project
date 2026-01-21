#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
#define PORT 9999 // 서버와 동일한 포트

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len;
    pid_t pid;

    // 1. 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    // 서버 주소 설정
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 로컬호스트(내 컴퓨터) IP
    serv_addr.sin_port = htons(PORT);

    // 2. 연결 요청
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");
    else
        puts("Connected to Server!");

    pid = fork(); // 두개로 나눠서 하나는 수신 하나는 발신

    if (pid == 0) // 수신(읽기)
    {
        while (1)
        {
            str_len = read(sock, message, BUF_SIZE - 1);

            if (str_len == 0 || str_len == -1)
            {
                break;
            }

            message[str_len] = 0;
            printf("Message from server: %s", message);
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
            write(sock, message, strlen(message));
        }
    }

    // 4. 종료
    close(sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}