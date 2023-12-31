#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
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
    int x, y, c, prex, prey;
    Cell block[BLOCK_SIZE][BLOCK_SIZE];
    struct timeval start_time, now_time, pre_time;
    double duration, thold;

    copyBlock(block_type[1], block);
    initialize();
    x = 5;
    y = 0;
    thold = 0.7; // 落下の時間感覚
    printBlock(block, x, y);
    gettimeofday(&start_time, NULL); // 開始時刻
    pre_time = start_time;
    for (y = 0; y < HEIGHT; ) {
        prex = x;
        prey = y;

        if (kbhit()) {
            clearBlock(block, x, y);
            c = getch(); // 1回目
            if (c == 0x1b) {
                c = getch(); // 2回目
                if (c == 0x5b) {
                    c = getch(); // 3回目
                    switch (c) {
                        case 0x41: // UP
                            break;
                        case 0x42: // DOWN
                            break;
                        case 0x43: // RIGHT
                            x++;
                            break;
                        case 0x44: // LEFT
                            x--;
                            break;
                        default:
                            exit(1);
                    }
                }
            }
            else { // 矢印キー以外の場合
                reset();
                exit(1);
            }
        }
        gettimeofday(&now_time, NULL);
        // 前回からの経過時間
        duration = now_time.tv_sec - pre_time.tv_sec
                + (now_time.tv_usec - pre_time.tv_usec) / 1000000.0;
        // もし落下時間間隔以上に時間経過していたら
        if (duration > thold) {
            pre_time = now_time;
            y++; // 自動的に1つ落下
        }
        // もしブロックが左右移動/落下していたら
        if (prex != x || prey != y) {
            // 全開位置のブロックを消去
            clearBlock(block, prex, prey);
            // 新しい場所に描画
            printBlock(block, x, y);
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
    if (tcgetattr(1, &otty) < 0)
        return -1;
    ntty = otty;
    // 入力フラグを設定
    ntty.c_iflag &= ~(INLCR|ICRNL|IXON|IXOFF|ISTRIP);
    // 出力フラグを設定
    ntty.c_oflag &= ~OPOST;
    // ローカルフラグを設定
    ntty.c_lflag &= ~(ICANON|ECHO);
    // 入力も最小文字数を1に
    ntty.c_cc[VMIN] = 1;
    // 入力待ちの時間を指定
    ntty.c_cc[VTIME] = 0;
    tcsetattr(1, TCSADRAIN, &ntty);
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
