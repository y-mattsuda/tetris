#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define clearScreen() printf("\033[2J")
// セル単位で座標をセット, (0,0)始まり
#define setPosition(x, y) printf("\033[%d;%dH",(y)+1,(x)*2+1)
#define setCharColor(n) printf("\033[3%dm",(n))
#define setBackColor(n) printf("\033[4%dm",(n))
#define setAttribute(n) printf("\033[%dm", (n))
#define cursorOn() printf("\033[?25h")
#define cursorOff() printf("\033[?25l")
// Color
#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7
#define DEFAULT 9
// Attribute
#define NORMAL 0
#define BLIGHT 1
#define DIM 2
#define UNDERBAR 4
#define BLINK 5
#define REVERSE 7
#define HIDE 8
#define STRIKE 9
// Field setting
#define WIDTH 10
#define HEIGHT 20
// Block
#define BLOCK_SIZE 4
#define BLOCK_NUM 7

extern int errno;

struct termios otty, ntty;

typedef struct cell {
    char c; // 表示文字
    int charColor; // 表示色
    int backColor; // 背景色
    int attribute; // 文字属性
} Cell;

// 7種類のブロックのマスタデータ
// 1種類のブロック形状に対して4*4の16個分のCellデータが16行分、それが7種類
Cell block_type[BLOCK_NUM][BLOCK_SIZE][BLOCK_SIZE] = {
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        ' ', RED, BLACK, REVERSE,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        ' ', RED, BLACK, REVERSE,
        ' ', RED, BLACK, REVERSE,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        ' ', RED, BLACK, REVERSE,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,
        '\0', RED, BLACK, NORMAL,

        '\0', BLUE, BLACK, NORMAL,
        ' ', BLUE, BLACK, REVERSE,
        ' ', BLUE, BLACK, REVERSE,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        ' ', BLUE, BLACK, REVERSE,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        ' ', BLUE, BLACK, REVERSE,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,
        '\0', BLUE, BLACK, NORMAL,

        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        ' ', GREEN, BLACK, REVERSE,
        ' ', GREEN, BLACK, REVERSE,
        '\0', GREEN, BLACK, NORMAL,
        ' ', GREEN, BLACK, REVERSE,
        ' ', GREEN, BLACK, REVERSE,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,
        '\0', GREEN, BLACK, NORMAL,

        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        ' ', YELLOW, BLACK, REVERSE,
        ' ', YELLOW, BLACK, REVERSE,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        ' ', YELLOW, BLACK, REVERSE,
        ' ', YELLOW, BLACK, REVERSE,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,
        '\0', YELLOW, BLACK, NORMAL,

        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        ' ', CYAN, BLACK, REVERSE,
        ' ', CYAN, BLACK, REVERSE,
        ' ', CYAN, BLACK, REVERSE,
        ' ', CYAN, BLACK, REVERSE,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,
        '\0', CYAN, BLACK, NORMAL,

        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        ' ', MAGENTA, BLACK, REVERSE,
        ' ', MAGENTA, BLACK, REVERSE,
        ' ', MAGENTA, BLACK, REVERSE,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        ' ', MAGENTA, BLACK, REVERSE,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,
        '\0', MAGENTA, BLACK, NORMAL,

        '\0', WHITE, BLACK, NORMAL,
        ' ', WHITE, BLACK, REVERSE,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        ' ', WHITE, BLACK, REVERSE,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        ' ', WHITE, BLACK, REVERSE,
        ' ', WHITE, BLACK, REVERSE,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
        '\0', WHITE, BLACK, NORMAL,
};

int kbhit(void); // キー入力があったかどうか確認
int getch(void); // キー入力1文字読み込み
int tinit(void); // 端末の初期化
void initialize(void); // 画面の初期化
void reset(void); // 画面の復元
int waitMsec(int msec);
int checkRange(Cell c, int x, int y);
int printCell(Cell c, int x, int y);
int clearCell(Cell c, int x, int y);
void copyBlock(Cell src[BLOCK_SIZE][BLOCK_SIZE], Cell dst[BLOCK_SIZE][BLOCK_SIZE]);
// ブロックの表示とクリア
// 座標(x,y)は4*4エリアの左上の座標とする
int printBlock(Cell block[BLOCK_SIZE][BLOCK_SIZE], int x, int y);
int clearBlock(Cell block[BLOCK_SIZE][BLOCK_SIZE], int x, int y);

int main() {
//    Cell block[BLOCK_SIZE][BLOCK_SIZE];
//    copyBlock(block_type[1], block);
//    initialize();
//    for (int y=0; y<HEIGHT-BLOCK_SIZE; y++) {
//        printBlock(block, 5, y);
//        waitMsec(500);
//        clearBlock(block, 5, y);
//    }
//    reset();
    int c;
    initialize();
    for (int count = 0; count < 10; ) {
        if (kbhit() != 0) {
            c = getch();
            printf("%x", c);
            count++;
        }
    }
    reset();
}

int kbhit(void) {
    int ret;
    fd_set rfd;
    struct timeval timeout = {0, 0};
    FD_ZERO(&rfd);
    FD_SET(0, &rfd); // 0:stdin
    ret = select(1, &rfd, NULL, NULL, &timeout);
    if (ret == 1)
        return 1;
    else
        return 0;
}

int getch(void) {
    unsigned char c;
    ssize_t n;
    while ((n = read(0, &c, 1)) < 0 && errno == EINTR);
    if (n == 0)
        return -1;
    else
        return (int)c;
}

static void onsignal(int sig) { // 内部利用のシグナルハンドラ
    signal(sig, SIG_IGN);
    switch (sig) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
        case SIGHUP:
            exit(1);
    }
}

int tinit(void) {
    setbuf(stdout, NULL);
    if (tcgetattr(STDIN_FILENO, &otty) < 0)
        return -1;
    ntty = otty;
    ntty.c_iflag &= ~(INLCR|ICRNL|IXON|ISTRIP|BRKINT);
    ntty.c_oflag &= ~OPOST;
    ntty.c_lflag &= ~(ICANON|ECHO|ECHONL|IEXTEN);
    ntty.c_cc[VMIN] = 1;
    ntty.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ntty) < 0)
        return -1;
    signal(SIGINT, onsignal);
    signal(SIGQUIT, onsignal);
    signal(SIGTERM, onsignal);
    signal(SIGHUP, onsignal);
    return 0;
}

void initialize(void) {
    tinit(); // 端末の初期化
    setBackColor(BLACK);
    setCharColor(WHITE);
    setAttribute(NORMAL);
    clearScreen();
    cursorOff();
}

void reset(void) {
    setBackColor(BLACK);
    setCharColor(WHITE);
    setAttribute(NORMAL);
    clearScreen();
    cursorOn();
    tcsetattr(1, TCSADRAIN, &otty); // 端末の復元
    write(1, "\n", 1); // 後始末
}

int waitMsec(int msec) {
    struct timespec r = {0, msec * 1000L * 1000L};
    return nanosleep(&r, NULL);
}

int checkRange(Cell c, int x, int y) {
    if (c.c=='\0' || x<0 || y<0 || x>=WIDTH || y>=HEIGHT)
        return -1;
    else
        return 0;
}

int printCell(Cell c, int x, int y) {
    if (checkRange(c, x, y) == -1)
        return -1;
    setPosition(x, y);
    setAttribute(c.attribute);
    setBackColor(c.backColor);
    setCharColor(c.charColor);
    printf("%c%c", c.c, c.c);
    fflush(stdout);
    return 0;
}

int clearCell(Cell c, int x, int y) {
    if (checkRange(c, x, y) == -1)
        return -1;
    setPosition(x, y);
    setAttribute(NORMAL);
    setBackColor(BLACK);
    setCharColor(BLACK);
    printf("  ");
    fflush(stdout);
    return 0;
}

void copyBlock(Cell src[BLOCK_SIZE][BLOCK_SIZE], Cell dst[BLOCK_SIZE][BLOCK_SIZE]) {
    for (int j = 0; j < BLOCK_SIZE; ++j)
        for (int i = 0; i < BLOCK_SIZE; ++i)
            dst[j][i] = src[j][i];
}

int printBlock(Cell block[BLOCK_SIZE][BLOCK_SIZE], int x, int y) {
    for (int j = 0; j < BLOCK_SIZE; ++j)
        for (int i = 0; i < BLOCK_SIZE; ++i)
            printCell(block[j][i], i + x, j + y);
    return 0;
}

int clearBlock(Cell block[BLOCK_SIZE][BLOCK_SIZE], int x, int y) {
    for (int j = 0; j < BLOCK_SIZE; ++j)
        for (int i = 0; i < BLOCK_SIZE; ++i)
            clearCell(block[j][i], i + x, j + y);
    return 0;
}
