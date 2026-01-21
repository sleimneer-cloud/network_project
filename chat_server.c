#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 1024
#define PORT 9999

void error_handling(char *message);

int main(int argc, char *argv[])
{
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i, j;
    char buf[BUF_SIZE];

    // 1. 서버 소켓 생성
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    // 2. select 준비 (관제탑 세팅)
    FD_ZERO(&reads);           // 목록 초기화
    FD_SET(serv_sock, &reads); // 서버 소켓(문지기) 등록
    fd_max = serv_sock;        // 가장 큰 소켓 번호 저장

    printf("Server started. Waiting for connections...\n");

    while (1)
    {
        cpy_reads = reads; // 원본 보존을 위해 복사!
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        // 3. 변화 감지 (누구 할 말 있는 사람?)
        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1)
            break;

        if (fd_num == 0)
            continue; // 타임아웃(아무 일 없음)

        // 4. 변화가 생긴 소켓들 확인
        for (i = 0; i < fd_max + 1; i++)
        {
            if (FD_ISSET(i, &cpy_reads)) // 변화가 있는 소켓 발견!
            {
                if (i == serv_sock) // Case A: 새로운 연결 요청
                {
                    adr_sz = sizeof(clnt_addr);
                    clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &adr_sz);

                    FD_SET(clnt_sock, &reads); // 감시 목록에 추가
                    if (fd_max < clnt_sock)
                        fd_max = clnt_sock; // max 갱신
                    printf("Connected client: %d\n", clnt_sock);
                }
                else // Case B: 채팅 메시지 도착 (또는 연결 종료)
                {
                    str_len = read(i, buf, BUF_SIZE);
                    if (str_len == 0) // 연결 종료 요청
                    {
                        FD_CLR(i, &reads); // 목록에서 삭제
                        close(i);          // 소켓 닫기
                        printf("Closed client: %d\n", i);
                    }
                    else // 메시지 도착 -> 모두에게 뿌리기 (Broadcast)
                    {
                        // 접속한 모든 소켓을 검사해서 메시지 전송
                        for (j = 0; j < fd_max + 1; j++)
                        {
                            // 목록에 있고, 서버가 아니고, 보낸 본인이 아니면 전송
                            if (FD_ISSET(j, &reads) && j != serv_sock && j != i)
                            {
                                write(j, buf, str_len);
                            }
                        }
                    }
                }
            }
        }
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}