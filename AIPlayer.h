#pragma once

class GomokuBoard;

struct TilePos {
	TilePos() : xPos(0), yPos(0) {}
	TilePos(int x, int y) : xPos(x), yPos(y) {}
	int xPos;
	int yPos;
};

class AIPlayer {
public:
	AIPlayer();
	~AIPlayer();

	TilePos getNextMove(GomokuBoard* gBoard);
	void setColor(int mColor);
	int getColor();

private:
	int mColor;
};