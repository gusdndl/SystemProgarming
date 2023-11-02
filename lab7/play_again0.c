#include <stdio.h>
#include <termios.h>

#define QUESTION "Do you wnat another transcation"

int get_response(char *question);

int main() {
    int response;
    response = get_response(QUESTION);
    return response;
}

int get_response(char *question) {
    printf("%s (y/n)?", question);
    while (1) {
        switch (getchar()) {
            case 'y':
            case 'Y':
                return 0;
            case 'n':
            case 'N':
            case EOF:
                return 1;
        }
    }
}