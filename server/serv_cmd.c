#include "common.h"

void normalize_path(char *path, char *root_dir);
int is_dir(const char *path);

int change_dir(int clnt_sock, client_t *clnt_info){ //cd
    char buf[CWD_LEN], target[MAXLINE], new_path[1024];
    char root_dir[CWD_LEN] = "/home/pi/workspace/2025-capstone-digital-signature/server/file";
    int status = 0;
    memset(buf, 0x00, CWD_LEN);
    memset(new_path, 0x00, 1024);
    send(clnt_sock, clnt_info->cwd, CWD_LEN, 0);

    recv(clnt_sock, buf, CWD_LEN, 0);
    //printf("%s\n", path);
   
    sscanf(buf + 2, "%s", target);
    //printf("%s\n", target);

    if(target[0] == '/') {//절대경로 -> / 로시작
        snprintf(new_path, sizeof(new_path), "%s%s", clnt_info->cwd, target);
    }else {
        snprintf(new_path, sizeof(new_path), "%s/%s", clnt_info->cwd, target);
    }
    //printf("2%s\n", new_path);
    normalize_path(new_path, root_dir);

    size_t root_len = strlen(root_dir);

    if(strncmp(new_path, root_dir, root_len) != 0) {
        status = 1;
        send(clnt_sock, &status, sizeof(int), 0);
        return -1;
    }
    //printf("3%s\n", new_path);
    // 존재 여부 확인
    if(!is_dir(new_path)){
        status = 2;
        send(clnt_sock,  &status, sizeof(int), 0);
        return -1;
    }
    //printf("4%s\n", new_path);
    strncpy(clnt_info->cwd, new_path, CWD_LEN);
    send(clnt_sock, &status, sizeof(int), 0);
}

//server <- client 파일 다운로드
int clnt_put(int client_fd, char *buffer, char *command, EVP_PKEY *pub_key, char *path){
    int check, fd, file_len, bytes_left, file_size, total_len= 0;
    int success = 1;
    size_t sign_len;
    char file_data[BUFFER_SIZE], filename[MAXLINE], file_buf[BUFFER_SIZE], sign_buff[100], full_path[BUFFER_SIZE];

    memset(file_data, 0x00, BUFFER_SIZE);

    sscanf(buffer + strlen(command), "%s", filename); //command 이후 filename에 포인팅
    //printf("filename: %s\n", filename);

    while(1){
        snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
        fd = open(full_path, O_CREAT | O_EXCL | O_WRONLY, 0666);
        if(fd == -1){
            sprintf(filename + strlen(filename), "_1");}
        else
            break;
    }

    //printf("\n=======[데이터 수신 시작]=======\n");
    //printf("\n");

    recv(client_fd, &file_size, sizeof(int), 0);	//파일의 전체 크기 수신
    bytes_left = file_size;
    
    int cnt = 1;
    while(bytes_left > 0){ //클라이언트에서 받은 파일 크기만큼 반복문수행
        //printf("Fragment %d\n", cnt);
        Length_Info info;
        memset(file_buf, 0x00, BUFFER_SIZE);
        memset(sign_buff, 0x00, 100);
        sign_len = 0;
        total_len = 0;

        recv(client_fd, &info, sizeof(Length_Info), 0); //파일 길이, 서명길이, 총길이 데이터를 담은 구조체 recv
        
        file_len = info.file_len;
        sign_len = info.sign_len;
        total_len = info.total_len;

        //printf("\t파일 길이: (%d) || 디지털 서명 길이: (%zu)\n", file_len, sign_len);
        //printf("\t총 패킷 길이: %d\n", total_len);

        //수신용 버퍼 동적 생성
        unsigned char *recv_buf = (unsigned char *)malloc(total_len);
        if(recv_buf == NULL) {
            perror("malloc failed");
            success =0;
            break;
        }

        int recv_bytes = recv(client_fd, recv_buf, total_len, 0); //자른 파일 데이터 + 데이터에 대한 서명 값 recv
        if(recv_bytes != total_len){
            perror("send failed");
            success =0;
            break;
        }

        memcpy(file_buf, recv_buf, file_len);
        memcpy(sign_buff, recv_buf + file_len, sign_len);

        printf("\n");
        

        if(ecdsa_verify(file_buf, file_len, sign_buff, sign_len, pub_key)){ //서명 검증
            printf("[서명 검증]--->");
            printf("\tverification success\n");
            check = write(fd, file_buf, file_len);	//검증 성공시 파일 데이터 write
        }else{
            printf("[서명 검증]--->");
            printf("\tverification fail\n");
            success = 0;
        }

        if(check < 0){
            perror("파일 쓰기 오류 발생: \n");
            success = 0;
        }

        bytes_left -= file_len; //수신한 파일의 크기에서 recv한 데이터 크기만큼 빼서 남은 파일 크기 계산
        free(recv_buf);
        
        //printf("\n");
        //printf("--------------------------------\n");
        //printf("\n");
        cnt++;

        //printf("success value : %d\n", success);
        send(client_fd, &success, sizeof(int), 0); //fragment당 디지털 서명 검증에 성공하였는지 client에게 송신 0: 실패 1: 성공
        sleep(1);

         if(!success){
            break;
        }

    }
    printf("\n");
    if(file_len < 0){
        perror("파일 수신 오류 발생: \n");
        success = 0;
    }

    close(fd);
    
    if(success){
        printf("%s / save success\n", filename);
    }else{
        printf("%s / save fail\n", filename);
        remove(full_path); //검증이 실패했거나 파일 write, 수신에 오류가 발생시 파일 삭제
    }

    //send(client_fd, &success, sizeof(int), 0);		//write 성공 여부를 client 송신

    //printf("\n");
    //printf("=======[데이터 수신 끝]=========\n\n");
}

//clietn -> server 클라이언트에 있는 파일 다운로드
int clnt_get(int client_fd, char *buffer, char  *command, char *path){
    struct stat obj;
    size_t sign_len;
    int fd, status, file_size, bytes_send, total_len;
    char file_data[BUFFER_SIZE], filename[MAXLINE], full_path[BUFFER_SIZE], file_buf[BUFFER_SIZE];
    unsigned char *sign;
    total_len, bytes_send, status = 0;

    memset(file_data, 0x00, BUFFER_SIZE);
    memset(full_path, 0x00, BUFFER_SIZE);
    
    sscanf(buffer + strlen(command), "%s", filename); //command 이후 filename에 포인팅
    //printf("filename: %s\n", filename); //확인용 나중에 주석처리

    snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
    fd = open(full_path, O_RDONLY);

    if(fd == -1){//파일 존재 여부
        send(client_fd, &status, sizeof(int), 0); //요구한 파일이 없을 경우
        return -1;
    }else{
        status = 1;
        send(client_fd, &status, sizeof(int), 0);
    }

    stat(full_path, &obj);   //파일 크기
    file_size = obj.st_size;	//stat 명령를 통해 파일 사이즈 받기
    //printf("File_size: %d byte\n\n", file_size); //확인용

    send(client_fd, &file_size, sizeof(int), 0); //파일 크기 전송

    while((bytes_send = read(fd, file_buf, BUFFER_SIZE)) >0){
        Length_Info info; //파일 길이, 서명길이, 총길이 데이터를 저장할 구조체 선언
        sign = NULL;
        sign_len = 0;

        ecdsa_sign(file_buf, bytes_send, &sign, &sign_len); //서명 동작

        total_len = (int)sign_len + bytes_send;

        info.sign_len = (int)sign_len;
        info.file_len = bytes_send;
        info.total_len = total_len;

        send(client_fd, &info, sizeof(Length_Info), 0); //파일 길이, 서명길이, 총길이 데이터를 담은 구조체 send

        unsigned char *send_buf = (unsigned char *)malloc(total_len);
        if(send_buf == NULL) {
            perror("malloc failed");
            status = 0;
            break;
        }

        memcpy(send_buf, file_buf, bytes_send);
        memcpy(send_buf+bytes_send, sign, sign_len);
        
        int sent_bytes = send(client_fd, send_buf, total_len, 0);
        if(sent_bytes != total_len){
            perror("send failed");
            status = 0;
            
        }
        free(send_buf);

        recv(client_fd, &status, sizeof(int), 0);	//상태 수신
        if(!status)
            break;
    }
    close(fd);

    recv(client_fd, &status, sizeof(int), 0);	//서버에서 받았는지 확인 메세지 수신
    if(status){//업로드 성공여부 판단
        printf("%s / upload success\n", filename);
    }else{
        printf("%s / upload fail\n", filename);
    }
}

int ls(int client_fd, char *path){
    char filename[BUFFER_SIZE], full_path[BUFFER_SIZE];
	DIR *d;
	struct dirent *dir;
	struct stat file_info;
	int status = 0;
    
	d = opendir(path);
	 while ((dir = readdir(d)) != NULL) {

        memset(filename, 0, BUFFER_SIZE);
        memset(full_path, 0, BUFFER_SIZE);

        snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);
        lstat(full_path, &file_info);

        // "." ".." 제외 (보통 ls에서 숨김)
        if(strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        // 파일 or 디렉토리 둘 다 전송
        if (S_ISDIR(file_info.st_mode)) {
            // 디렉토리는 뒤에 "/" 붙이기
            snprintf(filename, BUFFER_SIZE, "%s/", dir->d_name);
        }
        else if (S_ISREG(file_info.st_mode)) {
            snprintf(filename, BUFFER_SIZE, "%s", dir->d_name);
        }
        else {
            continue;  // 파일/폴더 외엔 무시
        }

        // status = 1 → 항목 있음
        status = 1;
        send(client_fd, &status, sizeof(int), 0);

        // 항상 MAXLINE 길이로 보내는 fixed-size 프로토콜
        send(client_fd, filename, BUFFER_SIZE, 0);
    }

    // 마지막에 status = 0 → 전송 끝
    status = 0;
    send(client_fd, &status, sizeof(int), 0);

    closedir(d);
    return 0;
}



int make_dir(int clnt_sock, char *buffer, char *command, char *path){
    int check_status = 0;
    char dir_name[MAXLINE], full_path[BUFFER_SIZE];
    struct stat st;

    sscanf(buffer + strlen(command), "%s", dir_name); //command 이후 dir name에 포인팅
    snprintf(full_path, sizeof(full_path), "%s/%s", path, dir_name);

    //1. 디렉토리 존재 여부 확인
    if (stat(full_path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            // 이미 디렉토리가 존재함
            check_status = 0;
        } else {
            // 같은 이름의 파일이 존재함
            check_status = -1;
        }
    } 
    else {
        //2. 디렉토리가 없음 → 생성 시도
        if (mkdir(full_path, 0755) == 0) {
            check_status = 1; // 성공적으로 생성
        } else {
            check_status = -1; // 생성 실패
        }
    }

    send(clnt_sock, &check_status, sizeof(int), 0);

    return 0;
}


void normalize_path(char *path, char *root_dir)
{
    char temp[CWD_LEN];
    char *token, *stack[64];
    int top = -1;

    strcpy(temp, path);

    token = strtok(temp, "/");
    while(token != NULL){
        if(strcmp(token, "..") == 0){
            if(top >= 0) top--; // 상위로 이동
        }
        else if(strcmp(token, ".") == 0){
            // 현재 디렉토리 → 무시
        }
        else if(strlen(token) > 0){
            stack[++top] = token;
        }
        token = strtok(NULL, "/");
    }

    // 재조합
    path[0] = '\0';
    strcat(path, "/");
    for(int i=0; i<=top; i++){
        strcat(path, stack[i]);
        if(i != top) strcat(path, "/");
    }
}

int is_dir(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

int serv_pwd(int clnt_sock, client_t *clnt_info){
    
    send(clnt_sock, clnt_info->cwd, CWD_LEN, 0);
    
}
//fail test
int clnt_get_test(int client_fd, char *buffer, char  *command, char *path){
    struct stat obj;
    size_t sign_len;
    int fd, test_fd, status, file_size, bytes_send, total_len;
    char file_data[BUFFER_SIZE], filename[MAXLINE], full_path[BUFFER_SIZE], file_buf[BUFFER_SIZE], test_file_buf[BUFFER_SIZE];
    unsigned char *sign;
    total_len, bytes_send, status = 0;

    memset(file_data, 0x00, BUFFER_SIZE);
    memset(full_path, 0x00, BUFFER_SIZE);
    
    sscanf(buffer + strlen(command), "%s", filename); //command 이후 filename에 포인팅
    //printf("filename: %s\n", filename); //확인용 나중에 주석처리

    snprintf(full_path, sizeof(full_path), "%s/%s", path, filename);
    fd = open(full_path, O_RDONLY);

    if(fd == -1){//파일 존재 여부
        send(client_fd, &status, sizeof(int), 0); //요구한 파일이 없을 경우
        return -1;
    }else{
        status = 1;
        send(client_fd, &status, sizeof(int), 0);
    }

    stat(full_path, &obj);   //파일 크기
    file_size = obj.st_size;	//stat 명령를 통해 파일 사이즈 받기
    //printf("File_size: %d byte\n\n", file_size); //확인용

    send(client_fd, &file_size, sizeof(int), 0); //파일 크기 전송

    /*----------------fail test---------------------------------------*/
    snprintf(full_path, sizeof(full_path), "./test_file/%s_test", filename);
    test_fd = open(full_path, O_RDONLY);
    /*----------------fail test---------------------------------------*/

    while((bytes_send = read(fd, file_buf, BUFFER_SIZE)) >0){
        //fail test
        read(test_fd, test_file_buf, BUFFER_SIZE);
        
        Length_Info info; //파일 길이, 서명길이, 총길이 데이터를 저장할 구조체 선언
        sign = NULL;
        sign_len = 0;

        ecdsa_sign(file_buf, bytes_send, &sign, &sign_len); //서명 동작

        total_len = (int)sign_len + bytes_send;

        info.sign_len = (int)sign_len;
        info.file_len = bytes_send;
        info.total_len = total_len;

        send(client_fd, &info, sizeof(Length_Info), 0); //파일 길이, 서명길이, 총길이 데이터를 담은 구조체 send

        unsigned char *send_buf = (unsigned char *)malloc(total_len);
        if(send_buf == NULL) {
            perror("malloc failed");
            status = 0;
            break;
        }
        //fail test
        memcpy(send_buf, test_file_buf, bytes_send);
        memcpy(send_buf+bytes_send, sign, sign_len);
        
        int sent_bytes = send(client_fd, send_buf, total_len, 0);
        if(sent_bytes != total_len){
            perror("send failed");
            status = 0;
        }
        free(send_buf);
        recv(client_fd, &status, sizeof(int), 0);	//상태 수신
        if(!status)
            break;
    }
    close(fd);
    close(test_fd);

    recv(client_fd, &status, sizeof(int), 0);	//상태 수신
    if(status){//업로드 성공여부 판단
        printf("%s / upload success\n", filename);
    }else{
        printf("%s / upload fail\n", filename);
    }
}
