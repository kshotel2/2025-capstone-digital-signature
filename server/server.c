//server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024
//#define SERVER_IP "127.0.0.1"
int main() {
    int server_fd, client_fd, fd, file_size;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
	char buffer[BUFFER_SIZE], command[5], filename[256], buf[BUFFER_SIZE];

  	// 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){perror("socket"); exit(1);}

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 					//inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
	server_addr.sin_addr.s_addr = INADDR_ANY;		
    server_addr.sin_port = htons(PORT);

    // 3. 바인드
    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("bind"); close(server_fd); exit(1);}
	
	while(1){
    	// 4. 리슨
    	if(listen(server_fd, 5) == -1){perror("listen"); close(server_fd);exit(1);}
    	printf("서버 대기 중...\n");

    	// 5. 연결 수락
    	client_len = sizeof(client_addr);
    	client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    	if(client_fd == -1){perror("accept"); close(server_fd); exit(1);}
    	printf("클라이언트 연결됨\n");

		while(1){
			memset(buffer, 0, BUFFER_SIZE);
			printf("명령 대기 중...\n");
		
			int recv_len = recv(client_fd, buffer, BUFFER_SIZE, 0); //명령어 이름 수신
			if(recv_len <= 0){perror("recv 실패"); break;}
	
			sscanf(buffer, "%s", command);	//명령어 command에 옮김

			printf("클라이언트로부터 명령 수신: %s\n", command);
			
			if(!strcmp(command, "exit")){ //exit 명령어
				printf("클라이언트 연결 종료\n");
				close(client_fd);
				break;
			}
			else if(!strcmp(command, "put")){
				int check = 0;
				char *file_data;

				sscanf(buffer + strlen(command), "%s", filename); //command 이후 filename에 포인팅
				printf("filename: %s\n", filename);
				while(1){
					fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);	
					if(fd == -1)
						sprintf(filename + strlen(filename), "_1");
					else
						break;
				}

				recv(client_fd, &file_size, sizeof(int), 0);	//클라이언트에서 수행한 명령의 출력의 길이를 받음
				file_data = malloc(file_size);					//사이즈 만큼 동적할당

				recv(client_fd, buf, file_size, 0);				//클라이언트에서 명령 수행 결과를 받아옴
				check = write(fd, buf, file_size);				//위에서 연 파일에 write
				printf("%s save success\n", filename);			
				close(fd);
				free(file_data);

				send(client_fd, &check, sizeof(int), 0);		//write 성공 여부를 client 송신
			}
    	}	
	}
    // 7. 종료
    close(client_fd);
    close(server_fd);
    return 0;
}
/*
        	int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
			if (bytes_read <= 0) break;
        	printf("수신: %s\n", buffer);
        	// 클라이언트에게 그대로 되돌려보냄 (에코)
        	write(client_fd, buffer, bytes_read);
			*/	
