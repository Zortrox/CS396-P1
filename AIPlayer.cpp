/*
	AIPlayer.cpp
	Setup for the AI to make game moves

	@author Matthew Clark
	@date 09/02/2016
*/

#include "AIPlayer.h"
#include "GomokuBoard.h"
#include <stdlib.h>

AIPlayer::AIPlayer() {

}

AIPlayer::~AIPlayer() {
	//remove all tiles to clear RAM
	for (size_t i = 0; i < vecTileWeights.size(); i++) {
		delete vecTileWeights[i];
	}
}

void AIPlayer::init(GomokuBoard* gameBoard)
{
	//set up the board
	gBoard = gameBoard;
	size_t size = gBoard->getBoardSize();

	//set last tile placed
	mLastTile = NULL;

	//create tile weight pointers
	vecTileGrid.resize(size);
	for (size_t i = 0; i < size; i++) {
		vecTileGrid[i].resize(size);
		for (size_t j = 0; j < size; j++) {
			vecTileGrid[i][j] = new TilePos(i, j, 0);
			vecTileWeights.push_back(vecTileGrid[i][j]);
		}
	}
}

void AIPlayer::reset() {
	//reset the weights of all tiles
	size_t size = vecTileWeights.size();
	for (size_t i = 0; i < size; i++) {
		vecTileWeights[i]->weight = 0;
	}
}

TilePos* AIPlayer::getNextMove()
{
	std::vector< std::vector <GameTile> > gameArea = gBoard->getGameArea();
	std::vector<TilePos> testTiles;
	
	//update and sort tiles by weight
	updateTileWeights();
	sortTiles();

	//select only the top, equal-weighted tiles
	int topWeight = vecTileWeights[0]->weight;
	std::vector<TilePos*> vecTilePicks;
	for (size_t i = 0; i < vecTileWeights.size(); i++) {
		if (vecTileWeights[i]->weight == topWeight) {
			vecTilePicks.push_back(vecTileWeights[i]);
		}
		else break;
	}

	//select a random tile from the top weights
	TilePos* placedTile = vecTilePicks[rand() % vecTilePicks.size()];
	placedTile->weight = 0;
	mLastTile = placedTile;
	return placedTile;
}

void AIPlayer::setPlayerNum(int num, int color)
{
	mPlayerNum = num;
}

int AIPlayer::getPlayerNum()
{
	return mPlayerNum;
}

int AIPlayer::getColor() {
	return mColor;
}

void AIPlayer::updateTileWeights()
{
	std::vector< std::vector<GameTile> > vecGameArea = gBoard->getGameArea();
	int boardSize = gBoard->getBoardSize();
	int inRow = 5;

	//check opponent last tile, then own last tile to get
	//both aggressive & defensive weights
	for (int player = 0; player < playerType::COUNT; player++) {
		int xCenter;
		int yCenter;
		int checkingColor;

		//check opponent
		if (player == playerType::OPPONENT) {
			GameTile* centerTile = gBoard->getLastStone();
			if (centerTile) {
				xCenter = centerTile->xGrid;
				yCenter = centerTile->yGrid;
				checkingColor = centerTile->color;
			}
			else continue;

			//set weight of just placed tile to 0
			vecTileGrid[xCenter][yCenter]->weight = 0;
		}
		//check self
		else if (player == playerType::SELF) {
			//get the last stone AI placed
			//if AI is first player, place a stone randomly
			//otherwise just use previous player's stone as weights
			GameTile* lastTile = gBoard->getLastStone();
			if (mLastTile) {
				xCenter = mLastTile->xGrid;
				yCenter = mLastTile->yGrid;
				checkingColor = mColor;
			}
			else if (!lastTile) {
				xCenter = rand() % (boardSize - 1);
				yCenter = rand() % (boardSize - 1);
				vecTileGrid[xCenter][yCenter]->weight = 2;
				continue;
			}
			else continue;
		}

		//check tiles in every direction from previous tile
		//and add to new weight
		for (int dir = 0; dir < 4; dir++) {
			//weights before and after (in row) new tile added
			int weightAfter = 0;
			int weightBeforeCenter = 0;
			int weightAfterCenter = 0;
			bool bCenterBlock = false;

			//change coordinates based on direction
			int xChange = 0;
			int yChange = 0;
			switch (dir) {
			case 0:
				xChange = 1;
				yChange = 0;
				break;
			case 1:
				xChange = 1;
				yChange = 1;
				break;
			case 2:
				xChange = 0;
				yChange = 1;
				break;
			case 3:
				xChange = -1;
				yChange = 1;
				break;
			}

			//check every tile in direction until non-checking color
			TilePos* endTile = NULL;
			for (int i = 0; i < 2 * inRow + 1; i++) {
				int currX = xCenter + i*xChange - inRow * xChange;
				int currY = yCenter + i*yChange - inRow * yChange;

				//if coordinate is on the board
				if (currX >= 0 && currX < boardSize && currY >= 0 && currY < boardSize) {
					int color = vecGameArea[currX][currY].color;

					//add to weight if spot isn't checking color
					if (color == checkingColor) {
						//set center block of stones flag
						//to set weight of adjacent empty squares
						if (currX == xCenter && currY == yCenter) {
							bCenterBlock = true;
							weightBeforeCenter = weightAfter;
						}

						weightAfter++;
					}
					//set block end points & update weight if length changed
					//(since center stone bridges gaps)
					else {
						//set weights if weights changed based on new stone being added
						if (endTile && bCenterBlock) {
							weightAfterCenter = weightAfter - weightBeforeCenter - 1;

							//add weight to end tile if block is empty
							if (vecGameArea[endTile->xGrid][endTile->yGrid].color == stoneColor::NONE) {
								endTile->weight += 2 * weightAfterCenter + 1 - (player == playerType::SELF);
								if (weightAfter == 4) {
									endTile->weight *= 3 + (player == playerType::SELF);
								}
							}
							//add weight to other end tile if block is empty
							if (vecGameArea[currX][currY].color == stoneColor::NONE) {
								vecTileGrid[currX][currY]->weight += 2 * weightBeforeCenter + 1 - (player == playerType::SELF);
								if (weightAfter == 4) {
									vecTileGrid[currX][currY]->weight *= 3 + (player == playerType::SELF);
								}
							}

							//set that the row isn't checking the center block of stones anymore
							bCenterBlock = false;
						}

						//restart weighing
						weightAfter = 0;
						endTile = vecTileGrid[currX][currY];
					}
				}
			}
		}
	}
}

void AIPlayer::sortTiles()
{
	TilePos* tempTile;
	int tempPos;

	//insertion sort tiles by weight: high to low
	for (size_t i = 1; i < vecTileWeights.size(); i++) {
		tempTile = vecTileWeights[i];
		tempPos = i - 1;

		//switches positions if weight is less than the the next one's weight
		while (tempPos >= 0 && vecTileWeights[tempPos]->weight < tempTile->weight) {
			vecTileWeights[tempPos + 1] = vecTileWeights[tempPos];
			tempPos--;
		}
		vecTileWeights[tempPos + 1] = tempTile;
	}
}
