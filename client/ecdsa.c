#include "common.h"


int ecdsa_sign(char *fileBuf, int len, unsigned char **signOut, size_t *signOutLen){
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    
    //개인키 가져오기
    FILE *fp = fopen("ec_private_key.pem", "r");
    if(!fp){
        perror("개인 키 파일 열기 실패");
        return -11;
    }
    EVP_PKEY *pkey = PEM_read_PrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);

    if(!pkey){
        fprintf(stderr, "개인 키 로딩 실패");
        return -12;
    }

    //ctx 초기화
    if(EVP_DigestSignInit(ctx, NULL, MdName, NULL, pkey) != 1){
        fprintf(stderr, "DigestSignInit 실패");
        EVP_MD_CTX_free(ctx);
        EVP_PKEY_free(pkey);
        return -13;
    }
    //파일값 해싱
    EVP_DigestSignUpdate(ctx, fileBuf, len);

    //서명 길이 유추 및 서명
    assert((*signOutLen = (size_t)EVP_PKEY_size(pkey))); //EVP_PKEY_size -> pkey로 생성될 디지털 서명 최대 길이 바이트 단위로 알려준다
    *signOut = OPENSSL_malloc(*signOutLen); //서명 길이만큼 동적 할당
    assert(EVP_DigestSignFinal(ctx, *signOut, signOutLen)); //EVP_DigestSignUpdate 기반으로 서명 결과를 signOut에 채우고 길이를 signOutLen에 실제 사용된 바이트 수 반환
    
    EVP_MD_CTX_free(ctx);
    EVP_PKEY_free(pkey);

}