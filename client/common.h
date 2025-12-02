#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
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

#define BUFFER_SIZE 512
#define MAXLINE 256
#define MdName EVP_sha256()

int print_ls();
int ls(int sockfd, char *buffer);
int put_file(int sockfd);
int put_file_test(int sockfd);
int get_file(int sockfd, EVP_PKEY *pub_key);
int get_file_test(int sockfd, EVP_PKEY *pub_key);
int send_cert(int sockfd);
int clnt_to_serv(int sockfd);
int client_to_ca(int sockfd);
int ecdsa_sign(char *file_buf, int len, unsigned char **sign, size_t *sign_len);
int ecdsa_verify(char *file_buf, int len, unsigned char *sign, size_t sign_len, EVP_PKEY *pkey);
int cert_get_pubkey(int sockfd, EVP_PKEY **pkey);
int send_csr(int sockfd);
int save_cert(X509 *cert);
int clnt_make_dir();
int make_dir_to_serv(int sockfd);

typedef struct {
    int sign_len;
    int file_len;
    int total_len;
}Length_Info;
