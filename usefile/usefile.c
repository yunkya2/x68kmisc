#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("使用法: usefile <ファイル名>\n");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("ファイル '%s' を開けません: エラーコード %d\n", argv[1], errno);
        return 1;
    }

    printf("ファイル '%s' をオープンしました(fileno=%d)\n", argv[1], fd);

    system("");

    close(fd);
    printf("ファイル '%s' をクローズしました\n", argv[1]);
    return 0;
}
