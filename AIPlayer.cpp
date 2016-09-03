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
	mColor = color;
}

int AIPlayer::getPlayerNum()
{
	return mPlayerNum;
}

int AIPlayer::getColor() {
	return mColor;
}

std::vector<std::vector<TilePos*>> AIPlayer::getWeightGrid()
{
	return vecTileGrid;
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
				checkingColor = mColor;
				vecTileGrid[xCenter][yCenter]->weight = 100;
			}
			else continue;
		}

		//check tiles in every direction from previous tile
		//and add to new weight
		for (int dir = 0; dir < 4; dir++) {
			//weights for first and second empty tiles
			//before and after (in row) new tile added
			int weightBeforeFirst = 0;
			int weightAfterFirst = 0;

			int weightBeforeSecond = 0;
			int weightAfterSecond = 0;

			//if the center (new) tile has been checked
			bool bNewChecked = false;
			//keep weighing tiles 
			bool bKeepWeighing = true;
			//if current position is at the new tile placed
			bool bAtNewTile = false;

			//change relative coordinates based on direction
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
			TilePos* endTileFirst = NULL;
			TilePos* endTileSecond = NULL;
			for (int i = 0; i < 2 * inRow + 1; i++) {
				//current checking coordinates
				int currX = xCenter + i*xChange - inRow * xChange;
				int currY = yCenter + i*yChange - inRow * yChange;

				//flag if next block not on the board
				bool bNextOnBoard = true;
				int nextX = xCenter + (i + 1) * xChange - inRow * xChange;
				int nextY = yCenter + (i + 1) * yChange - inRow * yChange;
				if (nextX < 0 || nextX >= boardSize || nextY < 0 || nextY >= boardSize)
					bNextOnBoard = false;

				//flag if previous block not on the board
				bool bPrevOnBoard = true;
				int prevX = xCenter + (i - 1) * xChange - inRow * xChange;
				int prevY = yCenter + (i - 1) * yChange - inRow * yChange;
				if (prevX < 0 || prevX >= boardSize || prevY < 0 || prevY >= boardSize)
					bPrevOnBoard = false;

				//if coordinate is on the board
				if (currX >= 0 && currX < boardSize && currY >= 0 && currY < boardSize) {
					int color = vecGameArea[currX][currY].color;

					//set center block of stones flag
					//to set weight of adjacent empty squares
					if (currX == xCenter && currY == yCenter) {
						bNewChecked = true;
						bAtNewTile = true;
					}
					else bAtNewTile = false;

					//set this as the checking tile since
					//previous tile wasn't on the board
					if (!bPrevOnBoard) {
						endTileFirst = vecTileGrid[currX][currY];
					}

					//add to weight if spot is the checking color
					//or if spot is the center (for first stone placed)
					if (bKeepWeighing && (color == checkingColor || bAtNewTile)) {
						//increase weights depending on what
						//empty tile the AI has seen
						if (endTileSecond) {
							weightAfterSecond++;
						}
						else if (endTileFirst) {
							weightAfterFirst++;
						}
					}

					//check if need to set the tile weights based on the stone color/position
					if ((color != checkingColor && !bAtNewTile) || !bNextOnBoard) {

						//if another non-opponent tile is found or checking on the edge
						if ((endTileSecond || !bNextOnBoard) && bKeepWeighing) {

							//sets as endpoint if no second endpoint created yet
							if (!endTileSecond && !bNextOnBoard) {
								endTileSecond = vecTileGrid[currX][currY];
								weightBeforeSecond = weightAfterFirst;
							}

							//add weight to end tile if spot is empty & isn't the new tile placed
							if (vecGameArea[endTileFirst->xGrid][endTileFirst->yGrid].color == stoneColor::NONE && 
								(endTileFirst->xGrid != xCenter || endTileFirst->yGrid != yCenter)) {
								//total length of a row if stone is placed there
								int totalWeight = weightBeforeFirst + weightAfterFirst + 1;

								//update stone weight based on some more factors
								addWeightToTile(totalWeight, inRow, player, endTileFirst);
							}
							//add weight to other end tile if spot is empty & isn't the new tile placed
							if (vecGameArea[endTileSecond->xGrid][endTileSecond->yGrid].color == stoneColor::NONE &&
								(endTileSecond->xGrid != xCenter || endTileSecond->yGrid != yCenter)) {
								//total length of a row if stone is placed there
								int totalWeight = weightBeforeSecond + weightAfterSecond + 1;

								//update stone weight based on some more factors
								addWeightToTile(totalWeight, inRow, player, endTileSecond);
							}
							
							//stops weighing end tiles again
							bKeepWeighing = false;
						}

						//sets the ending as the first non-opponent colored space
						if (bNewChecked) {
							if (!endTileSecond) {
								endTileSecond = vecTileGrid[currX][currY];
								weightBeforeSecond = weightAfterFirst;
							}
						}
						//restart weighing if haven't checked the new tile yet
						else {
							weightBeforeFirst = weightAfterFirst;
							weightAfterFirst = 0;
							endTileFirst = vecTileGrid[currX][currY];
						}
					}
				}
			}
		}
	}
}

void AIPlayer::addWeightToTile(int totalWeight, int inRow, int player, TilePos* tileToSet)
{
	//increase the weight of certain tile moves more than others
	if (totalWeight == inRow && player == playerType::SELF) {
		totalWeight += 4;
	}
	else if (totalWeight == inRow && player == playerType::OPPONENT) {
		totalWeight += 3;
	}
	else if (totalWeight == inRow - 1 && player == playerType::SELF) {
		totalWeight += 2;
	}
	else if (totalWeight == inRow - 1 && player == playerType::OPPONENT) {
		totalWeight += 1;
	}

	if (totalWeight > tileToSet->weight) {
		tileToSet->weight = totalWeight;
	}
}

void AIPlayer::sortTiles() {
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
