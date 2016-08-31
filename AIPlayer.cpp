#include "AIPlayer.h"
#include "GomokuBoard.h"
#include <stdlib.h>
#include <vector>

AIPlayer::AIPlayer() {

}

AIPlayer::~AIPlayer() {

}

TilePos AIPlayer::getNextMove(GomokuBoard* gBoard)
{
	std::vector< std::vector <GameTile> > gameArea = gBoard->getGameArea();
	return TilePos(rand() % 15, rand() % 15);
}

void AIPlayer::setColor(int color)
{
	mColor = color;
}

int AIPlayer::getColor()
{
	return mColor;
}
