#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(int argc , char* argv) {
    int fd;
    char buf[BUFSIZ];

    if(argc != 2) {
        fprintf(stderr, "Usage: write0 ttyname\n");
        exit(1);
    }

    fd = open(argv[1], O_WRONLY);
    if(fd == -1) {
        perror(argv[1]);
        exit(1);
    }

    while(fgets(buf, BUFSIZ, stdin) != NULL) {
        if(write(fd, buf, strlen(buf)) == -1) {
            break;
        }
    }

    close(fd);

    return 0;
}