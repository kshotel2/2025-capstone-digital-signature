#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp = fopen("sample.txt", "w");
    if (fp == NULL) {
        perror("파일 열기 실패");
        return 1;
    }

    int target_size = 2000; // 2000바이트
    for (int i = 0; i < target_size; ++i) {
        // ASCII 문자 'A'~'Z' 반복적으로 작성
        char c = 'A' + (i % 26);
        fputc(c, fp);
    }

    fclose(fp);
    printf("sample.txt 파일을 2000바이트로 생성 완료.\n");
    return 0;
}
