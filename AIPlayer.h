#pragma once

#include <vector>

class GomokuBoard;

struct TilePos {
	TilePos() : xGrid(0), yGrid(0), weight(0) {}
	TilePos(int x, int y, int w) : xGrid(x), yGrid(y), weight(w) {}
	int xGrid;
	int yGrid;
	int weight;
};

class AIPlayer {
public:
	AIPlayer();
	~AIPlayer();

	void init(GomokuBoard* gameBoard);
	TilePos* getNextMove();
	void setColor(int mColor);
	int getColor();

private:
	void updateTileWeights();
	void sortTiles();

	int mColor;
	std::vector< std::vector<TilePos*> > vecTileGrid;
	std::vector<TilePos*> vecTileWeights;
	GomokuBoard* gBoard;
};