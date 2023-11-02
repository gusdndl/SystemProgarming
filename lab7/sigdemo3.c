#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

time_t time_s;
time_t time_e;
 void f(int signum);

 int main() {
    void f(int signum);
    signal(SIGINT, f);

    time_s = time(NULL);

    printf("you can't stop me!\n");

    while(1) {
        sleep(1);
        printf("haha\n");
    }

    return 0;
 }

 void f(int signum) {
    time_e = time(NULL);
    long int timeflow = time_e - time_s;
    printf("Currently elapsed time: %ld sec(s)!\n", timeflow);
 }