#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

time_t begin;

void f(int signum);

int main() {
    signal(SIGINT, f);
    begin = time(NULL);

    printf("you can't stop me\n");

    while(1) {
        sleep(1);
        printf("haha\n");
    }
    return 0;
}

void f(int signum) {
    time_t end = time(NULL);
    printf("Currently elapsed time: %ld sec(s)!\n", (end-begin));
}