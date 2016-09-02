/*
	GomokuBoard.h
	Actual game board for tile placement and win/tie checking

	@author Matthew Clark
	@date 09/02/2016
*/

#pragma once

#include <vector>
#include <string>

//tile colors on the board
namespace stoneColor {
	enum stoneColor { NONE, BLACK, WHITE };
}

//tiles objects on the board
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

	//adds a stone to a tile
	//returns if a stone was placed (no stone already there)
	bool addStone(int xPos, int yPos, int color, std::string entName, std::string nodeName);

	//returns if a stone is on a tile
	bool emptyTile(int xPos, int yPos);

	//returns if the board is full
	bool boardFilled();

	//returns if a game is won (row of 5 made)
	bool gameWon();

	//clears the board, sets all tiles to empty
	void clearBoard();

	//returns stone at tile position
	GameTile getStone(int xPos, int yPos);

	//returns last stone placed on board
	GameTile* getLastStone();

	//returns board size
	int getBoardSize();

	//returns all stones placed on the board
	std::vector<GameTile> getAllStones();

	//returns all tile spaces on the board
	std::vector< std::vector<GameTile> > getGameArea();

private:
	std::vector<std::vector<GameTile>> vecGameArea;	//2d array of all tiles
	GameTile* mLastTile;	//last tile placed
	int mBoardSize;			//width of the board (square board)
	int mTilesPlaced;		//total tiles placed so far
};