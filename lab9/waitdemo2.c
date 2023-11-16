#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define DELAY 2

void child_code(int delay);
void parent_code(int childpid);

int main() {
    int newpid;
    void child_code(), parent_code();

    printf("before: mypid is %d\n", getpid());

    if ((newpid = fork()) == -1) {
        perror("fork");
    } else if (newpid == 0) {
        child_code(DELAY);
    } else {
        parent_code(newpid);
    }

    return 0;
}

void child_code(int delay) {
    printf("child %d here. will sleep for %d seconds\n", getpid(), delay);
    sleep(delay);
    printf("child done. about to exit\n");
    exit(17);
}

void parent_code(int childpid) {
    int wait_rv;
    int child_status;
    int hight_8;
    int low_7;
    int bit_7;

    wait_rv = wait(&child_status);
    printf("done waiting for %d. Wait returned: %d\n", childpid, wait_rv);

    int mask;
    printf("Child status: ");
    for (int i = 15; i >= 0; i --) {
        mask = 1 << i;
        printf("%d", child_status & mask ? 1 : 0);
        if (i % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
    hight_8 = child_status >> 8;
    low_7 = child_status & 0x7F;
    bit_7 = child_status & 0x80;
    printf("status: exit=%d, sig=%d, core=%d\n", hight_8, low_7, bit_7);
}
