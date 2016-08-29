#include "GomokuBoard.h"

GomokuBoard::GomokuBoard()
{
	vecGameArea.resize(15);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(15);
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
		vecGameArea[xPos][yPos].color = color;
		vecGameArea[xPos][yPos].entName = entName;
		vecGameArea[xPos][yPos].nodeName = nodeName;

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

void GomokuBoard::clearBoard()
{
	vecGameArea.clear();

	vecGameArea.resize(15);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(15);
	}
}
