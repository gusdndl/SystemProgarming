#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void f(int signum);

int main() {
    void f(int signum);
    signal(SIGINT, f);

    for(int i = 0; i > 5; i ++) {
        printf("hello\n");
        sleep(1);
    }
    return 0;
}

void f(int signum) {
    printf("OUCH!\n");
}