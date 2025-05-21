//client.c
#include "common.h"

int main() {
    struct stat obj;
    int sockfd, fd, file_size, status;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE], filename[MAXLINE], buf[BUFFER_SIZE], file_buf[BUFFER_SIZE];

    // 1. 소켓 생성
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){perror("socket"); exit(1);}

    // 2. 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // 3. 서버에 연결
    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("connect"); close(sockfd); exit(1);}
    printf("서버에 연결됨\n");

    // 4. 데이터 송수신
    while(1){
        printf("명령어 입력 [put, exit](종료: exit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  

        if(strcmp(buffer, "exit") == 0){ //exit 명령어
			send(sockfd, buffer, 5, 0);
			printf("연결 종료\n");
			break;
		}	
        else if(strcmp(buffer, "put") == 0){ //put 명령어
            unsigned char *signOut;
            size_t signOutLen;

            int bytes_send = 0;

            printf("업로드 할 파일명을 입력해주세요 :");
            if(fgets(filename, sizeof(filename), stdin) == NULL){
		    	printf("입력 오류!\n");
		    	continue;
	    	}
            filename[strcspn(filename, "\n")] = 0;  // 엔터 제거
            
            printf("filename: %s\n", filename);
            if(strlen(filename) == 0){
                printf("파일명이 비어있습니다. 다시 입력해주세요.\n");
                continue;
            }
            
            if((fd = open(filename, O_RDONLY)) == -1){//파일 open
                printf("파일이 없습니다.\n");
                continue;
            }

            strcpy(buf, "put ");
            strcat(buf, filename);
            
            printf("명령어 조합 완료: %s\n", buf); 
            printf("명령어 전송 시작\n");
            send(sockfd, buf, BUFFER_SIZE, 0);//명령어 전송
            
            //파일 크기 
            stat(filename, &obj);
            file_size = obj.st_size;	//stat 명령를 통해 파일 사이즈 받기
            printf("파일 크기 : %d\n", file_size);

            send(sockfd, &file_size, sizeof(int), 0); //파일 크기 전송
            
            memset(file_buf, 0x00, BUFFER_SIZE);
            while((bytes_send = read(fd, file_buf, BUFFER_SIZE)) >0){

                hashFunction(file_buf, bytes_send);
     
                printf("서명시작\n");
                ecdsa_sign(file_buf, bytes_send, &signOut, &signOutLen);
                printf("서명끝 (%zu 바이트)\n",signOutLen);

                send(sockfd, file_buf, bytes_send, 0); //파일 전송

            }
            close(fd);

            recv(sockfd, &status, sizeof(int), 0);	//서버에서 받았는지 확인 메세지 수신
            if(status){//업로드 성공여부 판단
                printf("업로드 완료\n");
            }else{
                printf("업로드 실패\n");
            }
        }//end put  
	}
    //종료
    close(sockfd);
    return 0;
}
	 /*
        write(sockfd, buffer, strlen(buffer));
        memset(buffer, 0, BUFFER_SIZE);
        read(sockfd, buffer, BUFFER_SIZE - 1);
        printf("서버 응답: %s\n", buffer);
        */
