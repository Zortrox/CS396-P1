#include "GomokuBoard.h"

GomokuBoard::GomokuBoard()
{
	mLastTile = NULL;
	mBoardSize = 15;

	vecGameArea.resize(mBoardSize);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(mBoardSize);
	}
}

GomokuBoard::~GomokuBoard()
{
}

/*Places a stone if not already one there.
Returns whether a stone was created*/
bool GomokuBoard::addStone(int xPos, int yPos, int color, std::string entName, std::string nodeName)
{
	if (emptyTile(xPos, yPos)) {
		mLastTile = &vecGameArea[xPos][yPos];

		mLastTile->color = color;
		mLastTile->entName = entName;
		mLastTile->nodeName = nodeName;
		mLastTile->xGrid = xPos;
		mLastTile->yGrid = yPos;

		return true;
	}
	return false;
}

GameTile GomokuBoard::getStone(int xPos, int yPos)
{
	return vecGameArea[xPos][yPos];
}

bool GomokuBoard::emptyTile(int xPos, int yPos)
{
	return vecGameArea[xPos][yPos].color == stoneColor::NONE;
}

std::vector<GameTile> GomokuBoard::getAllStones()
{
	std::vector<GameTile> vecStones;

	for (size_t i = 0; i < vecGameArea.size(); i++) {
		for (size_t j = 0; j < vecGameArea[i].size(); j++) {
			if (!emptyTile(i, j)) vecStones.push_back(vecGameArea[i][j]);
		}
	}

	return vecStones;
}

bool GomokuBoard::gameWon() {
	int inRow = 5;		//number in row to win
	int xCenter = mLastTile->xGrid;
	int yCenter = mLastTile->yGrid;
	int lastColor = mLastTile->color;
	int xCurr = 0;
	int yCurr = 0;

	int iNum0 = 0;
	int iNum45 = 0;
	int iNum90 = 0;
	int iNum135 = 0;
	bool bWon = false;

	for (int i = 0; i < 2 * inRow - 1; i++) {
		xCurr = xCenter - (inRow - 1) + i;
		yCurr = yCenter;
		if (xCurr >= 0 && xCurr <= mBoardSize - 1) {
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum0++;
				if (iNum0 >= inRow) bWon = true;
			}
			else {
				iNum0 = 0;
			}
		}

		xCurr = xCenter - (inRow - 1) + i;
		yCurr = yCenter + (inRow - 1) - i;
		if (xCurr >= 0 && xCurr <= mBoardSize - 1 && yCurr >= 0 && yCurr <= mBoardSize - 1) {
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum45++;
				if (iNum45 >= inRow) bWon = true;
			}
			else {
				iNum45 = 0;
			}
		}

		xCurr = xCenter;
		yCurr = yCenter + (inRow - 1) - i;
		if (yCurr >= 0 && yCurr <= mBoardSize - 1) {
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum90++;
				if (iNum90 >= inRow) bWon = true;
			}
			else {
				iNum90 = 0;
			}
		}

		xCurr = xCenter + (inRow - 1) - i;
		yCurr = yCenter + (inRow - 1) - i;
		if (xCurr >= 0 && xCurr <= mBoardSize - 1 && yCurr >= 0 && yCurr <= mBoardSize - 1) {
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum135++;
				if (iNum135 >= inRow) bWon = true;
			}
			else {
				iNum135 = 0;
			}
		}
	}

	if (bWon) return true;
	else return false;
}

void GomokuBoard::clearBoard()
{
	mLastTile = NULL;

	vecGameArea.clear();
	vecGameArea.resize(mBoardSize);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(mBoardSize);
	}
}
