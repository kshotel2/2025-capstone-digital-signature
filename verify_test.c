#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

int main() {
    char filename[256];
    printf("검증할 메시지 파일 이름을 입력하세요: ");
    if (scanf("%255s", filename) != 1) {
        fprintf(stderr, "파일 이름 입력 실패\n");
        return 1;
    }

    // 1. 메시지 읽기
    FILE *msg_fp = fopen(filename, "rb");
    if (!msg_fp) {
        perror("메시지 파일 열기 실패");
        return 1;
    }

    fseek(msg_fp, 0, SEEK_END);
    long msg_len = ftell(msg_fp);
    rewind(msg_fp);

    unsigned char *message = malloc(msg_len);
    if (!message) {
        fprintf(stderr, "메모리 할당 실패\n");
        fclose(msg_fp);
        return 1;
    }

    fread(message, 1, msg_len, msg_fp);
    fclose(msg_fp);

    // 2. 서명 파일 읽기 (항상 message.sig라는 파일로 가정)
    FILE *sig_fp = fopen("message.sig", "rb");
    if (!sig_fp) {
        perror("message.sig 열기 실패");
        free(message);
        return 1;
    }

    fseek(sig_fp, 0, SEEK_END);
    long sig_len = ftell(sig_fp);
    rewind(sig_fp);

    unsigned char *signature = malloc(sig_len);
    if (!signature) {
        fprintf(stderr, "메모리 할당 실패\n");
        fclose(sig_fp);
        free(message);
        return 1;
    }

    fread(signature, 1, sig_len, sig_fp);
    fclose(sig_fp);

    // 3. 공개키 로드
    FILE *pub_fp = fopen("ecdsa_public.pem", "r");
    if (!pub_fp) {
        perror("ecdsa_public.pem 열기 실패");
        free(message);
        free(signature);
        return 1;
    }

    EVP_PKEY *pubkey = PEM_read_PUBKEY(pub_fp, NULL, NULL, NULL);
    fclose(pub_fp);
    if (!pubkey) {
        fprintf(stderr, "공개키 로딩 실패\n");
        free(message);
        free(signature);
        return 1;
    }

    // 4. 검증 컨텍스트 생성
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fprintf(stderr, "EVP_MD_CTX 생성 실패\n");
        EVP_PKEY_free(pubkey);
        free(message);
        free(signature);
        return 1;
    }

    if (EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, pubkey) <= 0) {
        fprintf(stderr, "DigestVerifyInit 실패\n");
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pubkey);
        free(message);
        free(signature);
        return 1;
    }

    // 5. 검증 수행
    int result = EVP_DigestVerify(mdctx, signature, sig_len, message, msg_len);

    if (result == 1) {
        printf("서명 검증 성공\n");
    } else if (result == 0) {
        printf("서명 검증 실패\n");
    } else {
        printf("서명 검증 중 오류\n");
    }

    // 정리
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pubkey);
    free(message);
    free(signature);

    return 0;
}
