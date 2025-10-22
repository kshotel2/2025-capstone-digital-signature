#include "common.h"

int ecdsa_verify(char *file_buf, int len, unsigned char *sign, size_t sign_len, EVP_PKEY *pkey){

    int ret = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_MD_CTX *tmp = EVP_MD_CTX_new(); //해쉬값 출력용

    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len = 0;

    if (!ctx) {
        fprintf(stderr, "EVP_MD_CTX_new 실패\n");
        return -1;
    }

    if (!pkey) {
        fprintf(stderr, "공개키가 NULL입니다\n");
        EVP_MD_CTX_free(ctx);
        return -2;
    }
	/*해싱값 출력쪽*/
    EVP_DigestInit_ex(tmp, EVP_sha256(), NULL);
    EVP_DigestUpdate(tmp, file_buf, len);
    EVP_DigestFinal_ex(tmp, digest, &digest_len);

    printf("SHA-256 digest:\t");
    for (unsigned int i = 0; i < digest_len; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");
	/*-----------------------------------------------------------*/

    //ctx 초기화
    if(EVP_DigestVerifyInit(ctx, NULL, MdName, NULL, pkey) != 1){
        fprintf(stderr, "DigestVerifyInit 실패");
        EVP_MD_CTX_free(ctx);

        return -13;
    }
   
    EVP_DigestVerifyUpdate(ctx, file_buf, len);

    ret = EVP_DigestVerifyFinal(ctx, sign, sign_len);
    EVP_MD_CTX_free(ctx);

    return ret;
}