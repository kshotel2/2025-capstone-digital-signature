#include "common.h"

int clnt_to_serv(int sockfd){
    char buffer[BUFFER_SIZE];
    EVP_PKEY *pub_key = NULL;

    //인증서 전송
    send_cert(sockfd);
    //서버의 인증서 수신후 CA에서 발급받았던 인증서였는지 검증후 공개키 추출
    cert_get_pubkey(sockfd, &pub_key);
    
    // 4. 데이터 송수신
    while(1){
        printf("명령어 입력 [put, get, ls, client_ls, mkdir, exit](종료: exit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if(strcmp(buffer, "exit") == 0){ //exit 명령어
			send(sockfd, buffer, 5, 0);
			printf("연결 종료\n");
			break;
		}else if(strcmp(buffer, "client_ls") == 0){ //file_ls명령어
            print_ls();
        }else if(strcmp(buffer, "put") == 0){ //put 명령어
            if(put_file(sockfd) == -1){
                continue;
            }
        }else if(strcmp(buffer, "get") == 0){ //put 명령어
            if(get_file(sockfd, pub_key) == -1){
                continue;
            }
        }else if(strcmp(buffer, "ls") == 0){// server쪽 ls 목록
            ls(sockfd, buffer);
        }else if(strcmp(buffer, "testput") == 0){ //put fail test
            if(put_file_test(sockfd) == -1){
                continue;
            }
        }else if(strcmp(buffer, "testget") == 0){// get fail test
            if(get_file_test(sockfd, pub_key) == -1){
                continue;
            }
        }else if(strcmp(buffer, "mkdir") == 0){

        }
	}
    return sockfd;
}