#include "common.h"

int hashFunction(char file_buf[BUFFER_SIZE], int file_length){
	
	EVP_MD_CTX *mdctx; //해쉬값을 저장할 구조체를 참조
	const EVP_MD *md;  //사용할 해쉬 함수 저장 
	int fd, size, md_len, i;

	unsigned char md_value[EVP_MAX_MD_SIZE]; //EVP_MD_CTX구조체에서 계산된 해시값을 담을 배열 
	

	if(file_buf == NULL){
		printf("hash error\n"); 
		exit(1);
	}
	
	//file hash	
	md = EVP_get_digestbyname("sha256"); //사용할 해시함수 선택 
	if(!md){
		printf("Unknown message digest \n");
		exit(2);
	}
	
	mdctx = EVP_MD_CTX_new(); //EVP_MD_CTX 생성 ctx : message digest context
	EVP_DigestInit_ex(mdctx, md, NULL); //ctx 초기화 
	
	//초기설정이 완료된 ctx로 buf에 있는 메시지를 buf의 길이만큼 해시 알고리즘을 수행
	EVP_DigestUpdate(mdctx, file_buf, file_length); 
	
        //ctx에 저장된 해시값을 md_value로 출력 출력된길이 md_len에 저장
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
        EVP_MD_CTX_free(mdctx);
	//file hash end
	
	//print hash value
	printf("Hash Digest is: ");
	for(i = 0; i < md_len; i++)
		printf("%02x", md_value[i]);
	
	printf("\n");	

  //  return md_value;

}
