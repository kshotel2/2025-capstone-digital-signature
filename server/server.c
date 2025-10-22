//server
#include "common.h"

#define PORT 12345

void* handle_clnt(void *arg);

int clnt_cnt = 0;
pthread_mutex_t mutx;

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
	char client_ip[INET_ADDRSTRLEN];
    socklen_t client_len;
	
	pthread_t t_id;//스레드

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
		

		while(1){
			client_len = sizeof(client_addr);
    		client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    		if(client_fd == -1){
				perror("accept"); 
				//close(server_fd); 
				//exit(1);
				continue;
			}
			int *pclient = malloc(sizeof(int));
			*pclient = client_fd;

			pthread_mutex_lock(&mutx);
			clnt_cnt++;
			printf("현재 접속한 클라이언트 개수 : %d\n", clnt_cnt);
			pthread_mutex_unlock(&mutx);

			pthread_create(&t_id, NULL, handle_clnt, (void*)pclient);
			pthread_detach(t_id);
		
			//inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    		//printf("[%s:%d 클라이언트 연결됨]\n", client_ip, PORT);


		}
    	// 5. 연결 수락
    	

		
	}
	
    // 7. 종료
    
    close(server_fd);
    return 0;
}

void* handle_clnt(void *arg){
		int clnt_sock = *(int*)arg;
		free(arg); //malloc했던 pclient 

		EVP_PKEY *pub_key = NULL;
		char buffer[BUFFER_SIZE], command[5];

		cert_get_pubkey(clnt_sock, &pub_key);
		//EVP_PKEY *pub_key = recv_pub_key(client_fd);
		send_cert(clnt_sock);
		while(1){
			memset(buffer, 0, BUFFER_SIZE);
			//printf("명령 대기 중...\n");
		
			int recv_len = recv(clnt_sock, buffer, BUFFER_SIZE, 0); //명령어 이름 수신
			if(recv_len <= 0){perror("recv 실패"); break;}
	
			sscanf(buffer, "%s", command);	//명령어 command에 옮김

			//printf("Client Command: %s\n", command);
			
			if(strcmp(command, "exit") == 0){ //exit 명령어
				printf("클라이언트 연결 종료\n");
				close(clnt_sock);
				break;
			}
			else if(strcmp(command, "put") == 0){	//put 명령어
				clnt_put(clnt_sock, buffer, command, pub_key);

			}else if(strcmp(command, "get") == 0){	//get 명령어
				clnt_get(clnt_sock, buffer, command);
			}else if(strcmp(command, "ls") == 0){
				ls(clnt_sock);
			}else if(strcmp(command, "testget") == 0){	//get 명령어
				clnt_get_test(clnt_sock, buffer, command);
			}
    	}
		EVP_PKEY_free(pub_key);
		close(clnt_sock);

		pthread_mutex_lock(&mutx);
		clnt_cnt--;
		printf("현재 접속한 클라이언트 개수 : %d\n", clnt_cnt);
		pthread_mutex_unlock(&mutx);

		return NULL;
}