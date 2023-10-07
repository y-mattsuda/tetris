#include <stdio.h>
#include <time.h>

#define clearScreen() printf("\033[2J")
#define setPosition(x, y) printf("\033[%d;%dH",(y),(x))
#define setCharColor(n) printf("\033[3%dm",(n))
#define setBackColor(n) printf("\033[4%dm",(n))
#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#define DEFAULT 9
#define setAttribute(n) printf("\033[%dm", (n))
#define NORMAL 0
#define BLIGHT 1
#define DIM 2
#define UNDERBAR 4
#define BLINK 5
#define REVERSE 7
#define HIDE 8
#define STRIKE 9

int wait(int msec);

int main() {
    int y;
    clearScreen();
    for (y=1; y<23; y++) {
        // セルを表示
        setPosition(5, y);
        setCharColor(WHITE);
        setBackColor(BLACK);
        setAttribute(REVERSE);
        printf("  ");
        fflush(stdout);
        wait(500); // 500msec待つ
        setPosition(5, y);
        setCharColor(WHITE);
        setBackColor(BLACK);
        setAttribute(NORMAL);
        printf("  ");
        fflush(stdout);
    }
    setBackColor(DEFAULT);
    setCharColor(DEFAULT);
    clearScreen();
}

int wait(int msec) {
    struct timespec r = {0, msec * 1000L * 1000L};
    return nanosleep(&r, NULL);
}
