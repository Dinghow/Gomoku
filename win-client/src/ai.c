#include "ai.h"

/*
 * YOUR CODE BEGIN
 * 你的代码开始
 */
 
//To count 8 direction's condition of a empty position
typedef struct
{
	struct Position ThisDotPosition;
	short int ChessDirection[8];
	struct Position border[8];
	short int layout[8][5];//To save one direction's blank and chesses condition
}DirectionCondition;

//To combine 2 direction's information to a line
typedef struct
{
	short int ChessLine[4];
	struct Position both_ends[4][2];
}LineCondition;

typedef struct
{
	int MyPoints[20][20];
	int OthersPoints[20][20];
}Points;

void initAI()
{

}

//Scan ther board
void Scanner(const char board[][20],int me, int MyPoints[][20])
{
	int other = (me == 1) ? 2 : 1;
	int count = 1;
	int i, j, k, t;
	DirectionCondition MyChess;
	int dx[8] = { 0,1,1,1,0,-1,-1,-1 };
	int dy[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	for (i = 0; i < 20; i++)
		for (j = 0; j < 20; j++)
		{
			if (board[i][j] == 0)
			{
				memset(MyChess.ChessDirection, 0, sizeof(MyChess.ChessDirection));
				memset(MyChess.layout, 0, sizeof(MyChess.layout));
				for (k = 0; k < 8; k++)
				{
					count = 1;
					MyChess.border[k].x = -50;
					MyChess.border[k].y = -50;
					int m = i, n = j, flag = 0;
					for (t = 0; t < 5; t++)
					{
						int p = m, q = n;
						m += dx[k];
						n += dy[k];
						if (m > 19 || n > 19 || m < 0 || n < 0 || board[m][n] == other)
						{
							MyChess.layout[k][t] = other;
							MyChess.border[k].x = p;
							MyChess.border[k].y = q;
							break;
						}
						else if (board[m][n] == me&&board[p][q] == 0 && (p != i || q != j))
						{
							MyChess.border[k].x = p;
							MyChess.border[k].y = q;
							MyChess.layout[k][t] = me;
							flag = 1;
						}
						else if ((flag == 0) && ((board[m][n] == me&&board[p][q] == me) || (board[m][n] == me&&p == i&&q == j)))
						{
							MyChess.layout[k][t] = me;
							MyChess.ChessDirection[k] = count++;
						}
						else if (board[m][n] == me)
							MyChess.layout[k][t] = me;
						else if (board[m][n] == 0 && board[p][q] != 0)
							MyChess.layout[k][t] = 0;
					}
				}
				MyChess.ThisDotPosition.x = i;
				MyChess.ThisDotPosition.y = j;
				MyPoints[i][j] = ((me == 1) ? 1 : 0.1)*PointsCalculate(board, me, MyChess);
			}
			else
				MyPoints[i][j] = -1;
		}
}
//Count points 
int PointsCalculate(const char board[BOARD_SIZE][BOARD_SIZE], int me, DirectionCondition Chesses)
{
	int other = (me == 1) ? 2 : 1;
	LineCondition onedot;
	memset(onedot.ChessLine, 0, sizeof(onedot.ChessLine));
	int onedotpoints = 0;
	int k, flag3 = 0, flag4 = 0;
	int xn = Chesses.ThisDotPosition.x, yn = Chesses.ThisDotPosition.y;

	//Combine directions' information to lines
	for (k = 0; k < 4; k++)
	{
		onedot.ChessLine[k] = Chesses.ChessDirection[k] + Chesses.ChessDirection[k + 4];
		onedot.both_ends[k][0] = Chesses.border[k];
		onedot.both_ends[k][1] = Chesses.border[k + 4];
	}

	//Analyze every line
	//连续型
	//竖列及第二个方向使用y坐标判断
	for (k = 0; k < 2; k++)
	{
		//分别表示两边的可落子空数
		int dy1 = abs(onedot.both_ends[k][0].y - yn) - Chesses.ChessDirection[k], dy2 = abs(onedot.both_ends[k][1].y - yn) - Chesses.ChessDirection[k + 4];
		//无界情况转化,除落子后5连外dy至少大于等于1
		if (onedot.both_ends[k][0].y < 0)
			dy1 = 4 - Chesses.ChessDirection[k];
		if (onedot.both_ends[k][1].y < 0)
			dy2 = 4 - Chesses.ChessDirection[k + 4];
		//连续型的判断
		//五连
		if (onedot.ChessLine[k] + 1 >= 5)
			onedotpoints += 10000000;
		//四连
		else if (onedot.ChessLine[k] + 1 == 4)
		{
			//落子后为活四
			if (dy1 >= 1 && dy2 >= 1)
				onedotpoints += 400000;
			//落子后为单边四
			if ((dy1 == 0 && dy2 >= 1) || (dy2 == 0 && dy1 >= 1))
			{
				onedotpoints += 4000;
				flag4 += 1;
			}
			else
				onedotpoints += 0;
		}
		//三连
		else if (onedot.ChessLine[k] + 1 == 3)
		{
			//活三
			if (dy1 >= 1 && dy2 >= 1)
			{
				onedotpoints += 6000;
				flag3 += 1;
			}
			//单边三
			else if ((dy1 == 0 && dy2 >= 2) || (dy1 >= 2 && dy2 == 0))
				onedotpoints += 50;
			else
				onedotpoints += 0;
		}
		//二连
		else if (onedot.ChessLine[k] + 1 == 2)
		{
			//活二
			if ((dy1 + dy2 >= 3) && dy1 > 0 && dy2 > 0)
			{
				onedotpoints += 700;
				if (me == 2)
					onedotpoints -= 630;
			}
			//单边二
			if ((dy1 == 0 && dy2 >= 3) || (dy2 == 0 && dy1 >= 3))
				onedotpoints += 20;
			else
				onedotpoints += 0;
		}
	}
	//横行和第四个方向用x坐标判断
	for (k = 2; k < 4; k++)
	{
		int dx1 = abs(onedot.both_ends[k][0].x - xn) - Chesses.ChessDirection[k], dx2 = abs(onedot.both_ends[k][1].x - xn) - Chesses.ChessDirection[k + 4];
		if (onedot.both_ends[k][0].x < 0)
			dx1 = 4 - Chesses.ChessDirection[k];
		if (onedot.both_ends[k][1].x < 0)
			dx2 = 4 - Chesses.ChessDirection[k + 4];
		if (onedot.ChessLine[k] + 1 >= 5)
			onedotpoints += 10000000;
		else if (onedot.ChessLine[k] + 1 == 4)
		{
			if (dx1 >= 1 && dx2 >= 1)
				onedotpoints += 400000;
			if ((dx1 == 0 && dx2 >= 1) || (dx2 == 0 && dx1 >= 1))
			{
				flag4 += 1;
				onedotpoints += 4000;
			}
			else
				onedotpoints += 0;
		}
		else if (onedot.ChessLine[k] + 1 == 3)
		{
			if (dx1 >= 1 && dx2 >= 1)
			{
				flag3 += 1;
				onedotpoints += 6000;
			}
			else if ((dx1 == 0 && dx2 >= 2) || (dx1 >= 2 && dx2 == 0))
				onedotpoints += 50;
			else
				onedotpoints += 0;
		}
		else if (onedot.ChessLine[k] + 1 == 2)
		{
			if ((dx1 + dx2 >= 3) && dx1 > 0 && dx2 > 0)
			{
				onedotpoints += 700;
				if (me == 2)
					onedotpoints -= 630;
			}
			if ((dx1 == 0 && dx2 >= 3) || (dx2 == 0 && dx1 >= 3))
				onedotpoints += 20;
			else
				onedotpoints += 0;
		}
	}
	//隔空型
	for (k = 0; k < 4; k++)
	{
		//活三
		//第一种
		if ((Chesses.layout[k + 4][0] == 0 && Chesses.layout[k][0] == 0 && Chesses.layout[k][1] == me&&Chesses.layout[k][2] == me&&Chesses.layout[k][3] == 0) || (Chesses.layout[k][0] == 0 && Chesses.layout[k + 4][0] == 0 && Chesses.layout[k + 4][1] == me&&Chesses.layout[k + 4][2] == me&&Chesses.layout[k + 4][3] == 0))
		{
			flag3 += 1;
			onedotpoints += 4900;
		}
		//第二种
		if ((Chesses.layout[k + 4][0] == 0 && Chesses.layout[k + 4][1] == me && Chesses.layout[k + 4][2] == 0 && Chesses.layout[k][0] == me&&Chesses.layout[k][1] == 0) || (Chesses.layout[k][0] == 0 && Chesses.layout[k][1] == me && Chesses.layout[k][2] == 0 && Chesses.layout[k + 4][0] == me&&Chesses.layout[k + 4][1] == 0))
		{
			onedotpoints += 4400;
			flag3 += 1;
		}
		//第三种
		if ((Chesses.layout[k + 4][0] == me&&Chesses.layout[k + 4][1] == 0 && Chesses.layout[k + 4][2] == me&&Chesses.layout[k + 4][3] == 0 && Chesses.layout[k][0] == 0) || (Chesses.layout[k][0] == me&&Chesses.layout[k][1] == 0 && Chesses.layout[k][2] == me&&Chesses.layout[k][3] == 0 && Chesses.layout[k + 4][0] == 0))
		{
			flag3 += 1;
			onedotpoints += 4400;
		}
		//活三冲五
		if ((Chesses.layout[k + 4][0] == 0 && Chesses.layout[k + 4][1] == me&&Chesses.layout[k][0] == me&&Chesses.layout[k][1] == me &&Chesses.layout[k][2] == me) || (Chesses.layout[k][0] == 0 && Chesses.layout[k][1] == me&&Chesses.layout[k + 4][0] == me&&Chesses.layout[k + 4][1] == me &&Chesses.layout[k + 4][2] == me))
			onedotpoints += 500;
		//单边四
		if ((Chesses.layout[k][0] == 0 && Chesses.layout[k][1] == me&&Chesses.layout[k][2] == me&&Chesses.layout[k][3] == me&&Chesses.layout[k][4] == other) || (Chesses.layout[k + 4][0] == 0 && Chesses.layout[k + 4][1] == me&&Chesses.layout[k + 4][2] == me&&Chesses.layout[k + 4][3] == me&&Chesses.layout[k + 4][4] == other))
		{
			flag4 += 1;
			onedotpoints += 2900;
		}
	}
	//双活三
	if (flag3 == 2)
	{
		onedotpoints += 20000;
		if (me == 2)
			onedotpoints += 160000;
	}
	//双单边四
	if (flag4 == 2)
		onedotpoints += 800000;
	//单边四+活三
	if (flag3 == 1 && flag4 == 1)
		onedotpoints += 600000;

	return onedotpoints;
}
//Analyze every empty position
Points EmptyPositionAnalysis(const char board[BOARD_SIZE][BOARD_SIZE], int me)
{
	int other = (me == 1) ? 2 : 1;
	Points TotalPoints;
	memset(TotalPoints.MyPoints, 0, sizeof(TotalPoints.MyPoints));
	memset(TotalPoints.OthersPoints, 0, sizeof(TotalPoints.OthersPoints));

	//Count each side's chess condition
	Scanner(board,me, TotalPoints.MyPoints);
	Scanner(board,other, TotalPoints.OthersPoints);

	return TotalPoints;
}

struct Position aiBegin(const char board[BOARD_SIZE][BOARD_SIZE], int me){
	struct Position preferedPos;
	Points BothSidePoints = EmptyPositionAnalysis(board, me);
	int MyMax = 0, OtherMax = 0;
	int i, j, m = 0, n = 0, p = 0, q = 0;

	//Find the max value
	MyMax = BothSidePoints.MyPoints[0][0];
	OtherMax = BothSidePoints.OthersPoints[0][0];
	for (i = 0; i < 20; i++)
		for (j = 0; j < 20; j++)
		{
			if (MyMax <= BothSidePoints.MyPoints[i][j])
			{
				MyMax = BothSidePoints.MyPoints[i][j];
				m = i;
				n = j;
			}
			if (OtherMax <= BothSidePoints.OthersPoints[i][j])
			{
				OtherMax = BothSidePoints.OthersPoints[i][j];
				p = i;
				q = j;
			}
		}
	preferedPos.x = (MyMax > OtherMax) ? m : p;
	preferedPos.y = (MyMax > OtherMax) ? n : q;

	return preferedPos;
}
struct Position aiTurn(const char board[BOARD_SIZE][BOARD_SIZE], int me,int otherX, int otherY)
{
	struct Position preferedPos;
	Points BothSidePoints = EmptyPositionAnalysis(board, me);
	int MyMax = 0, OtherMax = 0;
	int i, j, m = 0, n = 0, p = 0, q = 0;

	//Find the max value
	MyMax = BothSidePoints.MyPoints[0][0];
	OtherMax = BothSidePoints.OthersPoints[0][0];
	for (i = 0; i < 20; i++)
		for (j = 0; j < 20; j++)
		{
			if (MyMax <= BothSidePoints.MyPoints[i][j])
			{
				MyMax = BothSidePoints.MyPoints[i][j];
				m = i;
				n = j;
			}
			if (OtherMax <= BothSidePoints.OthersPoints[i][j])
			{
				OtherMax = BothSidePoints.OthersPoints[i][j];
				p = i;
				q = j;
			}
		}
	preferedPos.x = (MyMax > OtherMax) ? m : p;
	preferedPos.y = (MyMax > OtherMax) ? n : q;
	if (preferedPos.x == 19 && preferedPos.y == 19)
	{
		m = 10;
		n = 10;
		while (board[m++][n++] != 0);
		preferedPos.x = m - 1;
		preferedPos.y = n - 1;
	}

	return preferedPos;
}

/*
 * YOUR CODE END
 * 你的代码结束 
 */
