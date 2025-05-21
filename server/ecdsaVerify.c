#include "common.h"

int ecdsaVerify(char *fileBuf, int len, unsigned char *sign, size_t signLen){

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    
    //공개키 가져오기
    FILE *fp = fopen("ec_public_key.pem", "r");
    if(!fp){
        perror("공개 키 파일 열기 실패");
        return -11;
    }
    EVP_PKEY *pkey = PEM_read_PUBKEY(fp, NULL, NULL, NULL);
    fclose(fp);

    if(!pkey){
        fprintf(stderr, "공개 키 로딩 실패");
        return -12;
    }

    //ctx 초기화
    if(EVP_DigestVerifyInit(ctx, NULL, MdName, NULL, pkey) != 1){
        fprintf(stderr, "DigestVerifyInit 실패");
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return -13;
    }

    EVP_DigestVerifyUpdate(ctx, fileBuf, len);

    return(EVP_DigestVerifyFinal(ctx, sign, signLen));

    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

}