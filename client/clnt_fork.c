#include "common.h"

int print_ls(){
    pid_t pid = fork();
    int status;

    char cwd[CWD_LEN];
    getcwd(cwd, sizeof(cwd));
        

    if (pid < 0) {
        perror("fork 실패");
        return 1;
    } else if (pid == 0) {
        printf("\n");
        printf("----------파일 목록----------\n");
        printf("\n");

        // 자식 프로세스: ls 실행
        execl("/bin/ls", "ls", cwd, (char *)NULL);
        perror("execl 실패");  // execl 실패했을 때만 실행됨
        return 1;
    } else {
        waitpid(pid, &status, 0);
    
        if(WIFEXITED(status)){
            printf("\n");
            printf("-----------------------------\n");
            printf("\n");
            //정상종료
        }else{
            printf("ls명령 자식프로세스 비정상 종료\n");
        }
    }
    return 0;
}