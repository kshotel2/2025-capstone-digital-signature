//server
#include "common.h"

void* handle_clnt(void *arg);

int clnt_cnt = 0;
pthread_mutex_t mutx;

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
	char client_ip[INET_ADDRSTRLEN];
    socklen_t client_len;
	pthread_t t_id;//스레드
	

	if(argc != 2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

  	// 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == -1){perror("socket"); exit(1);}

	//서버 종료시 사용했던 port넘버가 TIME_WAIT상태여도 bind가능한 옵션
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    	perror("setsockopt");
    	exit(1);
	}

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET; 					//inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
	server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(atoi(argv[1]));

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
			//*pclient = client_fd;

			client_t *clnt_info = malloc(sizeof(client_t));
			clnt_info ->sock = client_fd;

			char cwd[MAXLINE];
			char full_path[BUFFER_SIZE];
			getcwd(cwd, sizeof(cwd));
			
			snprintf(full_path, sizeof(full_path), "%s/%s", cwd, "file");
			//printf("초기 cwd %s\n", full_path);
			strcpy(clnt_info->cwd, full_path); //초기 cwd
			clnt_info ->pub_key = NULL;

			pthread_mutex_lock(&mutx);
			clnt_cnt++;
			printf("현재 접속한 클라이언트 개수 : %d\n", clnt_cnt);
			pthread_mutex_unlock(&mutx);
			
			pthread_create(&t_id, NULL, handle_clnt, (void*)clnt_info);
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
		//int clnt_sock = *(int*)arg;
		client_t *clnt_info = (client_t*)arg;
		//free(arg); //*pclient

		int clnt_sock = clnt_info->sock;
		EVP_PKEY *pub_key = clnt_info->pub_key;
		char buffer[BUFFER_SIZE], command[10];
		//printf("clnt_sock : %d\n", clnt_sock);
		//접속한 클라이언트의 인증서를 수신후 CA에서 발급받았던 인증서인지 검증후 공개키 추출
		cert_get_pubkey(clnt_sock, &pub_key);
		

		//서버 인증서 접속한 클라이언트에게 송신
		send_cert(clnt_sock);

		while(1){
			memset(buffer, 0x00, BUFFER_SIZE);
			
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
				clnt_put(clnt_sock, buffer, command, pub_key, clnt_info->cwd);

			}else if(strcmp(command, "get") == 0){	//get 명령어
				clnt_get(clnt_sock, buffer, command, clnt_info->cwd);
			}else if(strcmp(command, "ls") == 0){
				ls(clnt_sock, clnt_info->cwd);
			}else if(strcmp(command, "testget") == 0){	//get 명령어
				clnt_get_test(clnt_sock, buffer, command, clnt_info->cwd);
			}else if(strcmp(command, "mkdir") == 0){
				make_dir(clnt_sock, buffer, command, clnt_info->cwd);
			}else if(strcmp(command, "cd") == 0){
				change_dir(clnt_sock, clnt_info);
			}else if(strcmp(command, "pwd") == 0){
				serv_pwd(clnt_sock, clnt_info);
			}
    	}
		EVP_PKEY_free(pub_key);
		close(clnt_sock);
		free(clnt_info);

		pthread_mutex_lock(&mutx);
		clnt_cnt--;
		printf("현재 접속한 클라이언트 개수 : %d\n", clnt_cnt);
		pthread_mutex_unlock(&mutx);

		return NULL;
}