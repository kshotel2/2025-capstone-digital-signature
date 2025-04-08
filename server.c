#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
	char buffer[BUFFER_SIZE];

  	// 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {perror("socket"); exit(1);}

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 				
    server_addr.sin_addr.s_addr = INADDR_ANY;		
    server_addr.sin_port = htons(PORT);

    // 3. 바인드
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_fd);
        exit(1);
    }
	while(1){
    	// 4. 리슨
    	if (listen(server_fd, 5) == -1) {
    	    perror("listen");
    	    close(server_fd);
        	exit(1);
    	}
    	printf("서버 대기 중...\n");

    	// 5. 연결 수락
    	client_len = sizeof(client_addr);
    	client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    	if (client_fd == -1) {
    	    perror("accept");
    	    close(server_fd);
    	    exit(1);
    	}
    	printf("클라이언트 연결됨\n");
		while(1){
    		// 6. 데이터 송수신
        	memset(buffer, 0, BUFFER_SIZE);
        	int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
			if (bytes_read <= 0) break;
        	printf("수신: %s\n", buffer);
        	// 클라이언트에게 그대로 되돌려보냄 (에코)
        	write(client_fd, buffer, bytes_read);
    	}	
	}

    // 7. 종료
    close(client_fd);
    close(server_fd);
    return 0;
}
