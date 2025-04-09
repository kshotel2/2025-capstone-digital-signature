//client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    //create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){perror("socket"); exit(1);}

    //server setting
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    //connect
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("connect"); close(sockfd); exit(1);}
	
    printf("Connected to Server\n");

    //communication
    while(1){
        printf("메시지 입력 (종료: exit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        if (strncmp(buffer, "exit", 4) == 0) break;

        write(sockfd, buffer, strlen(buffer));
        memset(buffer, 0, BUFFER_SIZE);
        read(sockfd, buffer, BUFFER_SIZE - 1);
        printf("서버 응답: %s\n", buffer);
    }
    close(sockfd);
    return 0;
}
	
