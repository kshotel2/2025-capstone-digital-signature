// ecdsa_sign_openssl3.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

int main() {
    char filename[256];
    printf("파일 이름을 입력하세요: ");
    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "파일 이름 입력 실패\n");
        return 1;
    }

    // 파일 열기
    FILE *msg_fp = fopen(filename, "rb");
    if (!msg_fp) {
        perror("파일 열기 실패");
        return 1;
    }

    // 파일 크기 구하기
    fseek(msg_fp, 0, SEEK_END);
    long message_len = ftell(msg_fp);
    rewind(msg_fp);

    if (message_len <= 0) {
        fprintf(stderr, "파일이 비어있거나 오류 발생\n");
        fclose(msg_fp);
        return 1;
    }

    // 메시지 읽기
    unsigned char *message = malloc(message_len);
    if (!message) {
        fprintf(stderr, "메모리 할당 실패\n");
        fclose(msg_fp);
        return 1;
    }

    fread(message, 1, message_len, msg_fp);
    fclose(msg_fp);

    unsigned char *sig = NULL;
    size_t sig_len = 0;

    // 1. 개인 키 로드
    FILE *fp = fopen("ecdsa_private.pem", "r");
    if (!fp) {
        perror("개인 키 파일 열기 실패");
        free(message);
        return 1;
    }

    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    if (!pkey) {
        fprintf(stderr, "개인 키 로딩 실패\n");
        free(message);
        return 1;
    }

    // 2. 서명 컨텍스트 생성
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fprintf(stderr, "EVP_MD_CTX 생성 실패\n");
        EVP_PKEY_free(pkey);
        free(message);
        return 1;
    }

    if (EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, pkey) <= 0) {
        fprintf(stderr, "DigestSignInit 실패\n");
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        free(message);
        return 1;
    }

    // 3. 서명 수행
    if (EVP_DigestSign(mdctx, NULL, &sig_len, message, message_len) <= 0) {
        fprintf(stderr, "서명 길이 계산 실패\n");
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        free(message);
        return 1;
    }

    sig = malloc(sig_len);
    if (!sig) {
        fprintf(stderr, "메모리 할당 실패\n");
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        free(message);
        return 1;
    }

    if (EVP_DigestSign(mdctx, sig, &sig_len, message, message_len) <= 0) {
        fprintf(stderr, "실제 서명 실패\n");
        free(sig);
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        free(message);
        return 1;
    }

    printf("서명 완료! (%zu 바이트)\n", sig_len);

    // 4. 서명 결과 저장
    FILE *sig_fp = fopen("message.sig", "wb");
    fwrite(sig, 1, sig_len, sig_fp);
    fclose(sig_fp);
    printf("서명 결과를 message.sig로 저장 완료\n");

    // 정리
    free(sig);
    free(message);
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);
    return 0;
}
