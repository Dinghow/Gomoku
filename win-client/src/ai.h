#ifndef AI
#define AI

#include "common.h"

void initAI();
struct Position aiBegin(const char board[BOARD_SIZE][BOARD_SIZE], int me);
struct Position aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me, int otherX, int otherY);

#endif
