//server
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

	//create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){perror("socket"); exit(1);}

    //server address setting
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 				
    server_addr.sin_addr.s_addr = INADDR_ANY;		
    server_addr.sin_port = htons(PORT);

    //bind
    if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("bind"); close(server_fd); exit(1);}
	
	while(1){
    	//listen
    	if(listen(server_fd, 5) == -1){perror("listen"); close(server_fd); exit(1);}
    	
		printf("서버 대기 중...\n");
		
    	//connect
    	client_len = sizeof(client_addr);
    	client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    	if(client_fd == -1){perror("accept"); close(server_fd); exit(1);}
		
    	printf("클라이언트 연결됨\n");
		while(1){
    		//communication
        	memset(buffer, 0, BUFFER_SIZE);
        	int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
			if (bytes_read <= 0) break;
        	printf("수신: %s\n", buffer);
        	write(client_fd, buffer, bytes_read);
    	}	
	}
    close(client_fd);
    close(server_fd);
    return 0;
}
