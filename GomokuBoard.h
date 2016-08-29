#pragma once

#include <vector>
#include <string>

enum stoneColor { NONE, BLACK, WHITE };

struct GameTile {
	GameTile() : color(stoneColor::NONE) {}
	int color;
	std::string entName;
	std::string nodeName;
};

class GomokuBoard {
public:
	GomokuBoard();
	~GomokuBoard();

	bool addStone(int xPos, int yPos, int color, std::string entName, std::string nodeName);
	GameTile getStone(int xPos, int yPos);
	bool emptyTile(int xPos, int yPos);
	std::vector<GameTile> getAllStones();
	void clearBoard();

private:
	std::vector<std::vector<GameTile>> vecGameArea;
};