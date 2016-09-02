/*
	GomokuBoard.cpp
	Actual game board for tile placement and win/tie checking

	@author Matthew Clark
	@date 09/02/2016
*/

#include "GomokuBoard.h"

GomokuBoard::GomokuBoard()
{
	//initializes board
	mLastTile = NULL;
	mBoardSize = 15;
	mTilesPlaced = 0;

	//creates stones at all tile positions
	vecGameArea.resize(mBoardSize);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(mBoardSize);
	}
}

GomokuBoard::~GomokuBoard()
{
}

bool GomokuBoard::addStone(int xPos, int yPos, int color, std::string entName, std::string nodeName)
{
	//if the tile is empty, add a stone with specified color
	if (emptyTile(xPos, yPos)) {
		mLastTile = &vecGameArea[xPos][yPos];

		mLastTile->color = color;
		mLastTile->entName = entName;
		mLastTile->nodeName = nodeName;
		mLastTile->xGrid = xPos;
		mLastTile->yGrid = yPos;

		mTilesPlaced++;

		return true;
	}
	//otherwise return that spot wasn't empty
	return false;
}

bool GomokuBoard::emptyTile(int xPos, int yPos)
{
	//returns if tile color is empty
	return vecGameArea[xPos][yPos].color == stoneColor::NONE;
}

bool GomokuBoard::boardFilled() {
	//returns if the number of current tiles is the total number of tiles
	return (mTilesPlaced == mBoardSize * mBoardSize);
}

bool GomokuBoard::gameWon() {
	int inRow = 5;		//number in row to win
	int xCenter = mLastTile->xGrid;		//last tile placed x position
	int yCenter = mLastTile->yGrid;		//last tile placed y position
	int lastColor = mLastTile->color;	//last tile placed color
	int xCurr = 0;		//current x position of check
	int yCurr = 0;		//current y position of check

	//NSEW -> cardinal directions
	int iNum0 = 0;		//number in  E-W  row
	int iNum45 = 0;		//number in SW-NE row
	int iNum90 = 0;		//number in  N-S  row
	int iNum135 = 0;	//number in NW-SE row

	bool bWon = false;	//if number of stones in row required to win

	//check 2 * inRow - 1 tiles for each row
	//ex. 5 inRow = 9 total tiles checked; 4 for each side of last tile placed
	for (int i = 0; i < 2 * inRow - 1; i++) {
		//check E-W row
		xCurr = xCenter - (inRow - 1) + i;
		yCurr = yCenter;
		//check if coordinates on the board
		if (xCurr >= 0 && xCurr <= mBoardSize - 1) {
			//add to row length if stone being checked is still the same color
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum0++;
				if (iNum0 >= inRow) bWon = true;
			}
			//otherwise reset number of stones in row
			else {
				iNum0 = 0;
			}
		}

		//check SW-NE row
		xCurr = xCenter - (inRow - 1) + i;
		yCurr = yCenter + (inRow - 1) - i;
		//check if coordinates on the board
		if (xCurr >= 0 && xCurr <= mBoardSize - 1 && yCurr >= 0 && yCurr <= mBoardSize - 1) {
			//add to row length if stone being checked is still the same color
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum45++;
				if (iNum45 >= inRow) bWon = true;
			}
			//otherwise reset number of stones in row
			else {
				iNum45 = 0;
			}
		}

		//check N-S row
		xCurr = xCenter;
		yCurr = yCenter + (inRow - 1) - i;
		//check if coordinates on the board
		if (yCurr >= 0 && yCurr <= mBoardSize - 1) {
			//add to row length if stone being checked is still the same color
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum90++;
				if (iNum90 >= inRow) bWon = true;
			}
			//otherwise reset number of stones in row
			else {
				iNum90 = 0;
			}
		}

		//check NW-SE row
		xCurr = xCenter + (inRow - 1) - i;
		yCurr = yCenter + (inRow - 1) - i;
		//check if coordinates on the board
		if (xCurr >= 0 && xCurr <= mBoardSize - 1 && yCurr >= 0 && yCurr <= mBoardSize - 1) {
			//add to row length if stone being checked is still the same color
			if (vecGameArea[xCurr][yCurr].color == lastColor) {
				iNum135++;
				if (iNum135 >= inRow) bWon = true;
			}
			//otherwise reset number of stones in row
			else {
				iNum135 = 0;
			}
		}
	}

	//if a row with inRow found, return that a win was found
	if (bWon) return true;
	//otherwise return no win
	else return false;
}

void GomokuBoard::clearBoard()
{
	//reset last tile placed and total tiles
	mLastTile = NULL;
	mTilesPlaced = 0;

	//reset all tiles on board to default
	vecGameArea.clear();
	vecGameArea.resize(mBoardSize);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(mBoardSize);
	}
}

GameTile GomokuBoard::getStone(int xPos, int yPos)
{
	return vecGameArea[xPos][yPos];
}

GameTile* GomokuBoard::getLastStone()
{
	return mLastTile;
}

int GomokuBoard::getBoardSize()
{
	return mBoardSize;
}

std::vector<GameTile> GomokuBoard::getAllStones()
{
	//search the board and return all tiles that aren't empty
	std::vector<GameTile> vecStones;
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		for (size_t j = 0; j < vecGameArea[i].size(); j++) {
			if (!emptyTile(i, j)) vecStones.push_back(vecGameArea[i][j]);
		}
	}

	return vecStones;
}

std::vector< std::vector<GameTile> > GomokuBoard::getGameArea()
{
	return vecGameArea;
}