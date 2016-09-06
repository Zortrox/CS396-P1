/*
	AIPlayer.h
	Setup for the AI to make game moves

	@author Matthew Clark
	@date 09/02/2016
*/

#pragma once

#include <vector>

class GomokuBoard;

//type of player helps when dealing with weights
namespace playerType {
	enum playerType { OPPONENT, SELF, COUNT };
}

//tile weight object
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

	//initialize the AI with the gameBoard
	void init(GomokuBoard* gameBoard);

	//reset tile weights and stones placed
	void reset();

	//get the nextmove based on tile
	TilePos* getNextMove();

	//set the player number (turn number)
	void setPlayerNum(int num, int color, bool firstPlayer);

	//get the player number
	int getPlayerNum();

	//get the stone color
	int getColor();

	std::vector< std::vector<TilePos*> > getWeightGrid();

private:
	//update the weights of the game board tiles to determine next move
	void updateTileWeights();

	//add weights to a tile based on aggression, defensiveness
	void addWeightToTile(int totalWeight, int inRow, int player, TilePos* tileToSet);

	//sort the tiles high -> low based on weights
	void sortTiles();

	int mPlayerNum;			//player (turn) number
	int mColor;				//stone color
	TilePos* mLastTile;		//last tile self placed
	std::vector< std::vector<TilePos*> > vecTileGrid;	//2d array of tiles on board
	std::vector<TilePos*> vecTileWeights;				//tiles sorted by weights
	GomokuBoard* gBoard;	//pointer to the game board
	bool mFirstPlayer;		//flag if first player (black)
};