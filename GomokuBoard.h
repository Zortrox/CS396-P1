#pragma once

#include <vector>
#include <string>

enum stoneColor { NONE, BLACK, WHITE };

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
	bool emptyTile(int xPos, int yPos);
	std::vector<GameTile> getAllStones();
	bool gameWon();
	void clearBoard();

private:
	std::vector<std::vector<GameTile>> vecGameArea;
	GameTile* mLastTile;
	int mBoardSize;
};