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


int send_cert(int sockfd);
int cert_get_pubkey(int client_fd, EVP_PKEY **pkey);
int ecdsa_verify(char *file_buf, int len, unsigned char *sign, size_t sign_len, EVP_PKEY *pkey);
int ecdsa_sign(char *file_buf, int len, unsigned char **sign, size_t *sign_len);
int clnt_put(int client_fd, char *buffer, char *command, EVP_PKEY *pub_key);
int clnt_get(int client_fd, char *buffer, char  *command);
int clnt_get_test(int client_fd, char *buffer, char *command);
int ls(int client_fd);
int make_dir(int clnt_sock, char *buffer, char *command);


typedef struct {
    int sign_len;
    int file_len;
    int total_len;
}Length_Info;