#pragma once

#include <vector>
#include <string>

namespace stoneColor {
	enum stoneColor { NONE, BLACK, WHITE };
}

struct GameTile {
	GameTile() : color(stoneColor::NONE) {}
	int color;
	std::string entName;
	std::string nodeName;
	int xGrid;
	int yGrid;
};

class GomokuBoard {
public:
	GomokuBoard();
	~GomokuBoard();

	bool addStone(int xPos, int yPos, int color, std::string entName, std::string nodeName);
	GameTile getStone(int xPos, int yPos);
	GameTile* getLastStone();
	bool emptyTile(int xPos, int yPos);
	int getBoardSize();
	std::vector<GameTile> getAllStones();
	std::vector< std::vector<GameTile> > getGameArea();
	bool gameWon();
	void clearBoard();

private:
	std::vector<std::vector<GameTile>> vecGameArea;
	GameTile* mLastTile;
	int mBoardSize;
};