#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_SIZE 1024

void error_handling(char *message);
void sort_strings(char *list[], int size);

int main(int argc, char *argv[]) {
    printf("About to run sort with input form %s\n", argv[1]);

    char temp[MAX_SIZE];
    char buf[MAX_SIZE][MAX_SIZE];
    char *list[MAX_SIZE];
    int cnt = 0;

    FILE *F;
    F = fopen(argv[1], "r");
    if(F == NULL) {
        error_handling("Failed to open file");
    } else {
        while (fgets(temp, sizeof(temp), F) != NULL) {
            strcpy(buf[cnt++], temp);
        }
    }
    fclose(F);

    for (int i = 0; i < cnt; i++) {
        buf[i][strlen(buf[i]) - 1] = 0;
        list[i] = buf[i];
    }
    
    sort_strings(list, cnt);

    for (int i = 0; i < cnt; i++) {
        printf("%s\n", list[i]);
    }
    printf("Done running sort < %s\n",argv[1]);
    return 0;
}

void sort_strings(char *list[], int size) {
    int min;
    char *temp;

    for (int i = 0; i < size - 1; i++) {
        min = i;
        for (int j = i + 1; j < size; j++) {
            if (strcmp(list[j], list[min]) < 0) {
                min = j;
            }
        }

        temp = list[i];
        list[i] = list[min];
        list[min] = temp;
    }
}

void error_handling(char *message) {
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
