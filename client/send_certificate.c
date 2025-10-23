#include "common.h"

int send_cert(int sockfd){
    X509 *cert = NULL;

    FILE *fp = fopen("./client_key/client_cert.pem", "r");
    if(!fp){
        perror("인증서 open 실패\n");
        return -14;
    }

    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);

    if(!cert){
        fprintf(stderr, "인증서 로딩 실패\n");
        return -15;
    }
    int len = i2d_X509(cert, NULL);
    if(len <0 ){
        fprintf(stderr, "인증서 길이 계산 실패\n");
        X509_free(cert);
        return -16;
    }
    
    unsigned char *buf = malloc(len);
    unsigned char *p = buf;
   
    i2d_X509(cert, &p); // X509 구조체 -> DER 인코딩

    send(sockfd, &len, sizeof(int), 0); //길이 전송
   
    send(sockfd, buf, len, 0); //DER 인코딩괸 인증서 전송

    free(buf);
    X509_free(cert);
    //printf("인증서 전송됨 :: 인증서 길이 : %d\n", len);
    return 0;
}

