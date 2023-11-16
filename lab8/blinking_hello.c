#include <stdio.h>
#include <curses.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    initscr();
    clear();

    while(1) {
        move(10,20);
        addstr("Hello, world");
        refresh();
        sleep(1);
        clear();
        move(10, 32);
        refresh();
        sleep(1);
    }

    endwin();
    return 0;
}