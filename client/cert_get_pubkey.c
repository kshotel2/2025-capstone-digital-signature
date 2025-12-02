#include "common.h"

X509 *open_cert();

int cert_get_pubkey(int sockfd, EVP_PKEY **pkey) {
    X509 *cert = NULL;
    X509 *ca_cert = open_cert();
    EVP_PKEY *ca_pub_key = X509_get_pubkey(ca_cert);
    int cert_len;

    if (recv(sockfd, &cert_len, sizeof(int), 0) <= 0) {
        perror("인증서 길이 수신 실패");
        return -1;
    }

    //printf("인증서 길이 : %d\n", cert_len);
    unsigned char *buf = malloc(cert_len);
    if (!buf) {
        perror("메모리 할당 실패");
        return -2;
    }
    
    recv(sockfd, buf, cert_len, 0);
    
    const unsigned char *p = buf;
    cert = d2i_X509(NULL, &p, cert_len);
    free(buf);

    if (!cert) {
        fprintf(stderr, "인증서 파싱 실패\n");
        return -4;
    }

    if(X509_verify(cert, ca_pub_key) == 1){
        //printf("서버 인증서 검증 성공 : CA가 서명한 인증서\n");
    }else{
        printf("서버 인증서 검증 실패\n");
        X509_free(cert);
        return -6;
    }

    *pkey = X509_get_pubkey(cert);
    if (*pkey == NULL) {
        fprintf(stderr, "공개키 추출 실패\n");
        X509_free(cert);
        return -5;
    }

    X509_free(cert);
    return 0;
}


X509 *open_cert(){
    X509 *cert = NULL;

    FILE *fp = fopen("./client_key/ca_cert.pem", "r");
    if(!fp){
        perror("ca 인증서 open 실패\n");
        return NULL;
    }

    cert = PEM_read_X509(fp, NULL, NULL, NULL);
    fclose(fp);

    return cert;
}