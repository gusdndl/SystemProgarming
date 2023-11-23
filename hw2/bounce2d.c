#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#define TOP_ROW    5
#define BOT_ROW   20
#define LEFT_EDGE 10
#define RIGHT_EDGE 70
#define TICKS_PER_SEC 50
#define BLANK ' '

struct ppball {
    int y_pos, x_pos,
        y_dir, x_dir;
};

struct ppball the_ball;

void set_up();
void wrap_up();
void bounce_or_lose(struct ppball *);
void ball_move(int);
int set_ticker(int);

int main()
{
    int c;
    set_up();

    while ((c = getchar()) != 'Q') {
        switch(c) {
            case 'f': the_ball.x_dir++; break;
            case 's': the_ball.x_dir--; break;
            case 'F': the_ball.y_dir++; break;
            case 'S': the_ball.y_dir--; break;
        }
    }

    wrap_up();
    return 0;
}

void set_up()
{
    the_ball.y_pos = TOP_ROW;
    the_ball.x_pos = LEFT_EDGE;
    the_ball.y_dir = 1;
    the_ball.x_dir = 1;

    signal(SIGINT, SIG_IGN);
    initscr();
    noecho();
    crmode();

    mvaddch(the_ball.y_pos, the_ball.x_pos, 'O');
    refresh();

    signal(SIGALRM, ball_move);
    set_ticker(1000 / TICKS_PER_SEC);
}

void wrap_up()
{
    set_ticker(0);
    endwin();
}

void bounce_or_lose(struct ppball *bp)
{
    if (bp->y_pos == TOP_ROW) {
        bp->y_dir = 1;
    } else if (bp->y_pos == BOT_ROW) {
        bp->y_dir = -1;
    }

    if (bp->x_pos == LEFT_EDGE) {
        bp->x_dir = 1;
    } else if (bp->x_pos == RIGHT_EDGE) {
        bp->x_dir = -1;
    }
}

void ball_move(int signum)
{
    signal(SIGALRM, SIG_IGN);
    mvaddch(the_ball.y_pos, the_ball.x_pos, BLANK);
    the_ball.y_pos += the_ball.y_dir;
    the_ball.x_pos += the_ball.x_dir;
    bounce_or_lose(&the_ball);
    mvaddch(the_ball.y_pos, the_ball.x_pos, 'O');
    refresh();

    signal(SIGALRM, ball_move);
}

int set_ticker(int n_msecs)
{
    struct itimerval new_timeset;
    long n_sec, n_usecs;

    n_sec = n_msecs / 1000;
    n_usecs = (n_msecs % 1000) * 1000L;

    new_timeset.it_interval.tv_sec = n_sec;
    new_timeset.it_interval.tv_usec = n_usecs;

    new_timeset.it_value.tv_sec = n_sec;
    new_timeset.it_value.tv_usec = n_usecs;

    return setitimer(ITIMER_REAL, &new_timeset, NULL);
}