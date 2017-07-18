#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <windows.h>
#include <string.h>
#include <iphlpapi.h>
#include <unistd.h>

#include "common.h"
#include "utils.h"
#include "ai.h"

#define START "START"
#define PLACE "PLACE"
#define BEGIN "BEGIN"
#define READY "READY"
#define TURN  "TURN" 
#define WIN   "WIN"
#define LOSE  "LOSE"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define LIST_SIZE 10
#define INFO_X 100
#define INFO_Y 1
#define MESSAGE_X 100
#define MESSAGE_Y 20

struct globalArgs_t {
    char *ip;
    int port;
    BOOL DEBUG;
    char *replayFile;
} globalArgs;

static const char *optString = "a:p:r:hD";

//Message and info pointer
struct pointer
{
    char str[51];
    int bgColor;
    int fgColor;
    struct pointer *prev;
    struct pointer *next;
};

//Replay pointer
struct rpointer
{
    int x;
    int y;
    struct rpointer *prev;
    struct rpointer *next;
};

const char *EMPTY_MESSAGE = "                                                  ";

struct pointer *infoList;
struct pointer *messageList;
struct rpointer *replayList;

char buffer[MAXBYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {0};
SOCKET sock;
HANDLE hin;
HANDLE hout;
int lastX, lastY;
int step = 0;

/*
 * Ip Utils
 */
 
BOOL isIp(const char *ip)
{
    int num;
    int flag = TRUE;
    int counter = 0;
    
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, ip);
    char *p = strtok(buffer, ".");
    
    while (p && flag)
    {
        num = atoi(p);
        
        if (num >= 0 && num <=255 && (counter++ < 4))
        {
            flag = TRUE;
            p = strtok(NULL, ".");
        }
        else
        {
            flag = FALSE;
            break;
        }
    }
    
    return flag && (counter == 4);
}

BOOL isPort(const int port)
{
    return (port >= 0 && port <= 65535);
}

char *getIp()
{
    PHOSTENT hostinfo;
    char name[255];
    char* ip;
    if(gethostname(name, sizeof(name)) == 0)
    {
        if((hostinfo = gethostbyname(name)) != NULL)
        {
            ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
            return ip;
        }
    }
    return NULL;
} 


/*
 * List Utils
 */
 
void insertStrToList(struct pointer **p, const char *str)
{
    *p = (*p)->prev;
    strcpy((*p)->str, str);
}
 
void initList(struct pointer **p)
{
    int i;
    struct pointer *head, *tail, *tp;
    head = (struct pointer *) malloc(sizeof(struct pointer));
    head->bgColor = 0;
    head->fgColor = 7;
    strcpy(head->str, EMPTY_MESSAGE);
    tail = head;
    
    for (i = 1; i < LIST_SIZE; i++)
    {
        tp = (struct pointer *) malloc(sizeof(struct pointer));
        tp->bgColor = 0;
        tp->fgColor = 7;
        strcpy(tp->str, EMPTY_MESSAGE);
        tp->next = head;
        head->prev = tp;
        head = tp;
    }
    
    head->prev = tail;
    tail->next = head;
    *p = head;
}

void initVars()
{
    initList(&infoList);
    initList(&messageList);
}

/*
 * UI Utils
 */
void setConsoleSize(int width, int height)
{
    system("mode con cols=180 lines=42");
}
 
/* Move cursor to specified position in the console */
void moveCursorTo(const int X, const int Y)
{
    COORD coord;
    coord.X = X;
    coord.Y = Y;
    SetConsoleCursorPosition(hout, coord);
}

/* 
 * Set background color and forground color 
 * See http://baike.baidu.com/item/SetConsoleTextAttribute
 */
void setColor(const int bg_color, const int fg_color)
{
    SetConsoleTextAttribute(hout, bg_color * 16 + fg_color);
}

/* Show cursor */ 
void showConsoleCursor(BOOL showFlag)
{
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hout, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(hout, &cursorInfo);
}

/* Show char at specified position in the console */
void showStrAt(const struct pointer *p, int x, int y)
{
    moveCursorTo(x, y);
    printf(EMPTY_MESSAGE);
    moveCursorTo(x, y);
    setColor(p->bgColor, p->fgColor);
    printf(p->str);
    setColor(0, 7);
}

/* Show info(Top-right) */
void showInfo(const char *info)
{
    insertStrToList(&infoList, info);
    infoList->bgColor = 0;
    infoList->fgColor = 7;
    struct pointer *p = infoList;
    int i;
    for (i = 0; i < LIST_SIZE; i++)
    {
        showStrAt(p, INFO_X, INFO_Y + i);
        p = p->next;
    }
}

/* Show colored info(Top-right) */
void showInfoWithColor(const char *info, int bgColor, int fgColor)
{
    insertStrToList(&infoList, info);
    infoList->bgColor = bgColor;
    infoList->fgColor = fgColor;
    struct pointer *p = infoList;
    int i;
    for (i = 0; i < LIST_SIZE; i++)
    {
        showStrAt(p, INFO_X, INFO_Y + i);
        p = p->next;
    }
}

/* Show message(Bottom-right)*/ 
void showMessage(const char *message)
{
    insertStrToList(&messageList, message);
    messageList->bgColor = 0;
    messageList->fgColor = 7;
    struct pointer *p = messageList;
    int i;
    for (i = 0; i < LIST_SIZE; i++)
    {
        showStrAt(p, MESSAGE_X, MESSAGE_Y + i);
        p = p->next;
    }
}

/* Reset board */ 
void resetBoard()
{
    setColor(8, 0);
    moveCursorTo(0, 0); 
    
    printf("  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19\n");
    printf("A©³©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©·\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("B©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("C©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("D©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("E©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("F©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("G©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("H©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("I©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("J©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("K©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("L©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("M©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("N©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("O©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("P©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("Q©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("R©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("S©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("T©»©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©¿\n");
    
    /*
    printf("  0   1   2   3   4   5   6   7   8   9  10  11  12  13  14\n");
    printf("A©³©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©×©¥©·\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("B©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("C©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("D©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("E©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("F©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("G©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("H©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("I©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("J©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("K©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("L©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("M©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("N©Ç©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©ï©¥©Ï\n");
    printf(" ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§  ©§\n");
    printf("O©»©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©ß©¥©¿\n");
    */
    
    
    setColor(0, 7);
}

void initUI()
{
    hin = GetStdHandle(STD_INPUT_HANDLE);
    hout = GetStdHandle(STD_OUTPUT_HANDLE);
    
    showConsoleCursor(FALSE);
    
    setConsoleSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    resetBoard();
}

BOOL putChessAt(int x, int y)
{
    if (board[x][y] != EMPTY) return FALSE;
    
    if (step % 2 + 1 == BLACK)
    {
        board[x][y] = BLACK;
        setColor(8, 0);
    }
    else
    {
        board[x][y] = WHITE;
        setColor(8, 15);    
    }
    
    moveCursorTo(4 * x + 1, 2 * y + 1);
    printf("¡ñ");
    
    setColor(0, 7);
    ++step;
    
    return TRUE;
}

BOOL unPutChessAt(int x, int y)
{   
    moveCursorTo(4 * x + 1, 2 * y + 1);
    
    setColor(8, 0);
    if (x == 0 && y == 0)
    {
        printf("©³");
    }
    else if (x == 0 && y == BOARD_SIZE - 1)
    {
        printf("©·");
    }
    else if (x == BOARD_SIZE - 1 && y == 0)
    {
        printf("©»");
    }
    else if (x == BOARD_SIZE - 1 && y == BOARD_SIZE - 1)
    {
        printf("©¿");
    }
    else if (x == 0)
    {
        printf("©×");
    }
    else if (x == BOARD_SIZE - 1)
    {
        printf("©ß");
    }
    else if (y == 0)
    {
        printf("©Ç");
    }
    else if (y == BOARD_SIZE - 1)
    {
        printf("©Ï");
    }
    else
    {
        printf("©ï");
    }
    setColor(0, 7);
    
    
    board[x][y] = EMPTY;
    --step;
    
    return TRUE;
}


/*
 * Socket Utils
 */
void sendTo(const char *message, SOCKET *sock)
{
    send(*sock, message, strlen(message)+sizeof(char), NULL);
    Sleep(100);
}

void startSock()
{
    //Init socket DLL 
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    initSocketBuffer();
}

void initSock(const char *ip, const int port)
{
    //Open socket
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //Connect to socket
    struct sockaddr_in sockAddr;
    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = inet_addr(ip);
    sockAddr.sin_port = htons(port);
    
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "Trying to connect to %s:%d", ip, port);
    showInfo(buffer);
    while (connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR)))
    {
        showInfoWithColor("Connect failed, retry after 5s...\n", 0, FOREGROUND_RED);
        sleep(5);
    }
    
    showInfoWithColor("Connected\n", 0, FOREGROUND_GREEN);
}

void closeSock()
{
    //Close socket
    closesocket(sock);
    
    //Close socket DLL
    WSACleanup();
}

void start()
{
    memset(board, 0, sizeof(board));
    step = 0;
    lastX = 0;
    lastY = 0;
    initAI();
}

void begin()
{
    INPUT_RECORD ir[128];
    DWORD nRead;
    COORD xy;
    UINT i;
    SetConsoleMode(hin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
    
    if (TRUE == globalArgs.DEBUG) {
        /*
         * For debug
         */
        while (TRUE)
        {
            ReadConsoleInput(hin, ir, 128, &nRead);
            for (i = 0; i < nRead; i++)
            {
                if (MOUSE_EVENT == ir[i].EventType && FROM_LEFT_1ST_BUTTON_PRESSED == ir[i].Event.MouseEvent.dwButtonState)
                {
                    int rawX = ir[i].Event.MouseEvent.dwMousePosition.X;
                    int rawY = ir[i].Event.MouseEvent.dwMousePosition.Y;
                    
                    if (rawX % 4 == 0 || rawX % 4 == 3 || rawY % 2 == 0) continue;
                    
                    int x = rawX / 4;
                    int y = rawY / 2;
                    
                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && putChessAt(x, y))
                    {
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "%d %d\n", x, y);
                        sendTo(buffer, &sock);
                        return;
                    }
                }
            }
        }
    }
    else
    {
        /*
         * For AI
         */
         
        int flag = 0;
        struct Position pos;
        if (step % 2 + 1 == BLACK) pos = aiBegin((const char (*)[20])board, BLACK);
        else pos = aiBegin((const char (*)[20])board, WHITE);
        int x = pos.x;
        int y = pos.y;
        
        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && putChessAt(x, y))
        {
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%d %d\n", x, y);
            sendTo(buffer, &sock);
            return;
        }
    }
    
}

void ready()
{
    INPUT_RECORD ir[128];
    DWORD nRead;
    COORD xy;
    UINT i;
    SetConsoleMode(hin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
    
    if (TRUE == globalArgs.DEBUG) {
        /*
         * For debug
         */
        while (TRUE)
        {
            ReadConsoleInput(hin, ir, 128, &nRead);
            for (i = 0; i < nRead; i++)
            {
                if (MOUSE_EVENT == ir[i].EventType && FROM_LEFT_1ST_BUTTON_PRESSED == ir[i].Event.MouseEvent.dwButtonState)
                {
                    int rawX = ir[i].Event.MouseEvent.dwMousePosition.X;
                    int rawY = ir[i].Event.MouseEvent.dwMousePosition.Y;
                    
                    if (rawX % 4 == 0 || rawX % 4 == 3 || rawY % 2 == 0) continue;
                    
                    int x = rawX / 4;
                    int y = rawY / 2;
                    
                    if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && putChessAt(x, y))
                    {
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "%d %d\n", x, y);
                        sendTo(buffer, &sock);
                        return;
                    }
                }
            }
        }
    }
    else
    {
        /*
         * For AI
         */
         
        int flag = 0;
        struct Position pos;
        if (step % 2 + 1 == BLACK) pos = aiTurn((const char (*)[20])board, BLACK, lastX, lastY);
        else pos = aiTurn((const char (*)[20])board, WHITE, lastX, lastY);
        int x = pos.x;
        int y = pos.y;
        
        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && putChessAt(x, y))
        {
            memset(buffer, 0, sizeof(buffer));
            sprintf(buffer, "%d %d\n", x, y);
            sendTo(buffer, &sock);
            return;
        }
    }
    
}

void turn(int x, int y)
{
    putChessAt(x, y);
}

void win()
{
    showInfo("You win!\n");
}

void lose()
{
    showInfo("You Lose!\n");
}

void loop()
{
    while (TRUE)
    {
        memset(buffer, 0, sizeof(buffer));
        //Receive message from server
        recv(sock, buffer, MAXBYTE, NULL);
        
        addToSocketBuffer(buffer);
        
        while (hasCommand('\n'))
        {
            showMessage(socketArg);
            
            if (strstr(socketArg, START))
            {
                start();
            }
            else if (strstr(socketArg, PLACE))
            {
                char tmp[MAXBYTE] = {0};
                int x,  y;
                sscanf(socketArg, "%s %d %d", tmp, &x, &y);
                turn(x, y);
            }
            else if (strstr(socketArg, BEGIN))
            {
                begin();
            }
            else if (strstr(socketArg, READY))
            {
                ready();
            }
            else if (strstr(socketArg, TURN))
            {
                char tmp[MAXBYTE] = {0};
                int x,  y;
                sscanf(socketArg, "%s %d %d", tmp, &x, &y);
                turn(x, y);
                lastX = x;
                lastY = y;
            }
            else if (strstr(socketArg, WIN))
            {
                win();
            }
            else if (strstr(socketArg, LOSE))
            {
                lose();
            }
        
        }
        
    }
}

void initReplay()
{
    FILE *fp;
    
    replayList = (struct rpointer *) malloc(sizeof(struct rpointer));
    replayList->x = -1;
    replayList->y = -1;
    replayList->prev = NULL;
    replayList->next = NULL;

    
    struct rpointer *tail = replayList;
    struct rpointer *tp;
        
    if ((fp = fopen(globalArgs.replayFile, "r")) == NULL)
    {
        showInfo("Replay does not exist!");
        exit(1);
    }
    int x, y;
    while (fscanf(fp, "%d%d\n", &x, &y) != EOF)
    {
        tp = (struct rpointer *) malloc(sizeof(struct rpointer));
        tp->x = x;
        tp->y = y;
        tp->prev = tail;
        tp->next = NULL;
        tail->next = tp;
        tail = tp;
    }
}

void loopR()
{
    INPUT_RECORD ir[128];
    DWORD nRead;
    UINT i;
    SetConsoleMode(hin, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
    
    initReplay();
    
    struct rpointer *tp = replayList;
    
    while (TRUE)
    {
        ReadConsoleInput(hin, ir, 128, &nRead);
        for (i = 0; i < nRead; i++)
        {
            if (KEY_EVENT == ir[i].EventType)
            {
                if (ir[i].Event.KeyEvent.wVirtualKeyCode == VK_DOWN
                    && ir[i].Event.KeyEvent.bKeyDown == TRUE)
                {
                    // Down key
                    if (tp->next == NULL)
                    {
                        showInfo("Already at the last step!");
                    }
                    else
                    {
                        tp = tp->next;
                        turn(tp->x, tp->y);
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "%d step", step);
                        showInfo(buffer);
                    }
                }
                else if (ir[i].Event.KeyEvent.wVirtualKeyCode == VK_UP
                    && ir[i].Event.KeyEvent.bKeyDown == TRUE)
                {
                    // Up key
                    if (tp->prev == NULL)
                    {
                        showInfo("Already at the initial step!");
                    }
                    else
                    {
                        unPutChessAt(tp->x, tp->y);
                        tp = tp->prev;
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "%d step", step);
                        showInfo(buffer);
                    }
                }
            }
            
        }
    }
}

void display_usage(char *exe)
{
    printf("Usage: %s [OPTIONS] \n", exe);
    printf("  -a address        Server address\n");
    printf("  -p port           Server port\n");
    printf("  -D                Debug mode. When set, the user will manually play the chess.\n");
    printf("  -r replay         Replay mode. Should follow with a valid replay file.\n");
}

void initArgs(int argc, char *argv[])
{
    int opt = 0;
    globalArgs.DEBUG = FALSE;
    globalArgs.ip = getIp();
    globalArgs.port = 23333;
    globalArgs.replayFile = NULL;
    
    opt = getopt(argc, argv, optString);
    while (opt != -1)
    {
        switch (opt)
        {
            case 'a':
                globalArgs.ip = optarg;
                break;
            case 'p':
                globalArgs.port = atoi(optarg);
                break;
            case 'D':
                globalArgs.DEBUG = TRUE;
                break;
            case 'r':
                globalArgs.replayFile = optarg;
                break;
            case 'h':
                display_usage(argv[0]);
                exit(0);
                break;
            default:
                // Illegal!
                printf("Illegal arguments!\n");
                display_usage(argv[0]);
                exit(0);
                break;
        }
        
        opt = getopt(argc, argv, optString);
    }
    
    if (NULL != globalArgs.replayFile && access(globalArgs.replayFile, F_OK) == -1)
    {
        printf("Replay file is invalid! Exiting...\n");
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    startSock();
    
    initArgs(argc, argv);
    initVars();
    initUI();
    
    if (globalArgs.replayFile != NULL)
    {
        loopR();
    }
    else
    {
        initSock(globalArgs.ip, globalArgs.port);
        loop();
    }
    
    closeSock();

    return 0;
}
