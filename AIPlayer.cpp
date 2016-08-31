#include "AIPlayer.h"
#include "GomokuBoard.h"
#include <stdlib.h>

AIPlayer::AIPlayer() {

}

AIPlayer::~AIPlayer() {
	for (size_t i = 0; i < vecTileWeights.size(); i++) {
		delete vecTileWeights[i];
	}
}

void AIPlayer::init(GomokuBoard* gameBoard)
{
	gBoard = gameBoard;
	size_t size = gBoard->getBoardSize();

	vecTileGrid.resize(size);
	for (size_t i = 0; i < size; i++) {
		vecTileGrid[i].resize(size);
		for (size_t j = 0; j < size; j++) {
			vecTileGrid[i][j] = new TilePos(i, j, 0);
			vecTileWeights.push_back(vecTileGrid[i][j]);
		}
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
	return placedTile;
}

void AIPlayer::setColor(int color)
{
	mColor = color;
}

int AIPlayer::getColor()
{
	return mColor;
}

void AIPlayer::updateTileWeights()
{
	GameTile lastTile = gBoard->getLastStone();
	std::vector< std::vector<GameTile> > vecGameArea = gBoard->getGameArea();
	int boardSize = gBoard->getBoardSize();

	//set weight of just placed tile to 0
	vecTileGrid[lastTile.xGrid][lastTile.yGrid]->weight = 0;

	int inRow = 5;
	int xCenter = lastTile.xGrid;
	int yCenter = lastTile.yGrid;
	int oppColor = lastTile.color;

	//check tiles in every direction from previous tile
	//and add to new weight
	for (int dir = 0; dir < 4; dir++) {
		int weightAfter = 0;
		int weightBeforeCenter = 0;
		int weightAfterCenter = 0;
		bool bCenterBlock = false;

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

		//check every tile in direction until non-opponent color
		TilePos* endTile = NULL;
		for (int i = 0; i < 2 * inRow + 1; i++) {
			int currX = xCenter + i*xChange - inRow;
			int currY = yCenter + i*yChange - inRow;
			
			//if coordinate is on the board
			if (currX >= 0 && currX < boardSize && currY >= 0 && currY < boardSize) {
				int color = vecGameArea[currX][currY].color;

				//add to weight if spot is opponent color
				if (color == oppColor) {
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
					if (endTile && bCenterBlock) {
						weightAfterCenter = weightAfter - weightBeforeCenter - 1;

						//add to weight if it's an empty square
						if (vecGameArea[endTile->xGrid][endTile->yGrid].color == stoneColor::NONE) {
							endTile->weight += weightAfterCenter + 1;
						}
						if (vecGameArea[currX][currY].color == stoneColor::NONE) {
							vecTileGrid[currX][currY]->weight += weightBeforeCenter + 1;
						}

						bCenterBlock = false;
					}

					weightAfter = 0;
					endTile = vecTileGrid[currX][currY];
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

		while (tempPos >= 0 && vecTileWeights[tempPos]->weight < tempTile->weight) {
			vecTileWeights[tempPos + 1] = vecTileWeights[tempPos];
			tempPos--;
		}
		vecTileWeights[tempPos + 1] = tempTile;
	}
}
