#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <assert.h>
#include <arpa/inet.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/sha.h>

#define MdName EVP_sha256()
#define BUFFER_SIZE 512
#define MAXLINE 256
#define CWD_LEN 512

typedef struct { //접속한 유저 관리
    int sock;                 // 클라이언트 소켓
    char cwd[CWD_LEN];       // 클라이언트 전용 가상 cwd
    EVP_PKEY *pub_key;        // 클라이언트 공개키
    // int user_id
} client_t;

typedef struct {
    int sign_len;
    int file_len;
    int total_len;
}Length_Info;

int send_cert(int sockfd);
int cert_get_pubkey(int client_fd, EVP_PKEY **pkey);
int ecdsa_verify(char *file_buf, int len, unsigned char *sign, size_t sign_len, EVP_PKEY *pkey);
int ecdsa_sign(char *file_buf, int len, unsigned char **sign, size_t *sign_len);
int clnt_put(int client_fd, char *buffer, char *command, EVP_PKEY *pub_key, char *path);
int clnt_get(int client_fd, char *buffer, char  *command, char *path);
int clnt_get_test(int client_fd, char *buffer, char *command, char *path);
int ls(int client_fd, char *path);
int make_dir(int clnt_sock, char *buffer, char *command, char *path);
int change_dir(int clnt_sock, client_t *clnt_info);
int serv_pwd(int clnt_sock, client_t *clnt_info);


