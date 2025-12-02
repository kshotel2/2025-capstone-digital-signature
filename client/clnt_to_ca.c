#include "common.h"

int client_to_ca(int sockfd){
    
    char buffer[BUFFER_SIZE];

    /*-------------------------ca의 인증서를 받아 ca의 공개키 추출필요----------------------------------*/
    EVP_PKEY *ca_pub_key = NULL;
    cert_get_pubkey(sockfd, &ca_pub_key);
    printf("ca인증서 공개키 추출 완료\n");
    /*---------------------------------------------------------------------------------------------*/

    while(1){

        printf("명령어 입력 [request_cert, exit](종료: exit): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;  
        
        if(strcmp(buffer, "exit") == 0){ //exit 명령어
			send(sockfd, buffer, 5, 0);
			printf("연결 종료\n");
			break;
        
        }else if(strcmp(buffer, "request_cert") == 0){
            uint32_t net_len;

            send(sockfd, buffer, 13, 0);
            
            send_csr(sockfd);
            
            recv(sockfd, &net_len, sizeof(net_len), MSG_WAITALL); //MSG_WAITALL -> 정확히 요청한 길이만큼 다 받을떄까지 대기 
            uint32_t pem_len = ntohl(net_len);

            char *pem_buf = malloc(pem_len+1);
            recv(sockfd, pem_buf, pem_len, MSG_WAITALL);//인증서 정보 받아옴
            pem_buf[pem_len] = '\0';

            //pem -> X509구조체형식
            BIO *cbio = BIO_new_mem_buf(pem_buf, pem_len);
            X509 *cert = PEM_read_bio_X509(cbio, NULL, 0, NULL);

            if(cert){
                printf("클라이언트: 인증서 수신 성공\n");
                X509_print_fp(stdout, cert);
            }

            /*연결한 CA에서 발급한 인증서가 맞는지 검증*/
            if(X509_verify(cert, ca_pub_key) == 1){
                printf("검증 성공 : CA가 서명한 인증서\n");
            }else{
                printf("검증 실패\n");
                X509_free(cert);
            }

            save_cert(cert);

            BIO_free(cbio);
            free(pem_buf);

        }
    }
    return sockfd;
}