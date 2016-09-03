/*
	GomokuGame.h
	Sets up the game, input, display, and resources

	@author Matthew Clark
	@date 09/02/2016
*/

#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include <vector>
#include "Physics.h"
#include "GomokuBoard.h"
#include "AIPlayer.h"

//menu states for which menu buttons to show
namespace menuState {
	enum menuState { CLOSED, MAIN, NEW_GAME };
}
//menu button "names"
namespace menuButtons {
	enum menuButtons { B_QUIT, B_NEW, B_RESUME, B_VSAI, B_VSHUM, B_AIVAI, COUNT };
}
//the player colors
namespace gamePlayers {
	enum gamePlayers { P_BLACK, P_WHITE, COUNT };
}
//winner label "names"
namespace gameWinners {
	enum gameMessage { WIN_BLACK, WIN_WHITE, WIN_TIE, COUNT };
}

//this game extends these classes to override their functions for the Ogre graphics library
class GomokuGame : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
	GomokuGame();
    virtual ~GomokuGame();

	//starts the game
    virtual void go(void);

private:
	//setup main game
    bool setup();
	//configure how the window will display / graphics settings
    bool configure(void);
	//create the scene manager for nodes / entities
    void chooseSceneManager(void);
	//create camera
    void createCamera(void);
	//sets the frame listener, inputs, and GUI
    void createFrameListener(void);
	//sets up the scene nodes / entities
	void createScene(void);
	//create viewport where frame rendering will occur
    void createViewports(void);
	//setup + load resources (textures/materials/models)
    void setupResources(void);
	//runs before the frame is to be rendered
    bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	//---------------------------------------------------------------------------------
	//fires when a keyboard key is pressed
    bool keyPressed( const OIS::KeyEvent &arg );
	//fires when a keyboard key is released
    bool keyReleased( const OIS::KeyEvent &arg );
	//fires when the mouse moves
    bool mouseMoved( const OIS::MouseEvent &arg );
	//fires when a mouse button is pressed
    bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	//fires when a mouse button is released
    bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
	//---------------------------------------------------------------------------------
	
	//fires when a GUI button is pressed
	void buttonHit(OgreBites::Button* button);
	//sets the menu buttons based on the state
	void setMenu(int state);
	//sets the label based on the player's turn
	void setPlayerLabel(bool reset = false);

	//fires when the window is resized (update mouse clipping area)
    void windowResized(Ogre::RenderWindow* rw);
	//fires when the window is closed (shutdown OIS before window)
    void windowClosed(Ogre::RenderWindow* rw);

	//get the real world coordinates of where the player is looking
	Ogre::Vector3 getGameLookCoords();
	//set game stones to have physics & flip board
	void setStonePhysics();
	//add stone to board coordinates if one not already there
	bool addStoneToBoard(int xGrid, int yGrid);
	//add model + textures to new stone
	void addStoneGraphics(std::string strEntity, std::string strNode, int xGrid, int yGrid);
	//run the next turn for label switching & AI
	void nextTurn();
	//clear game data and reset to default, bring up new game menu
	void resetGame();
	//remove all menu items to display new meanu state
	void removeAllMenuItems(Ogre::OverlayContainer* menuContainer);
	//show winner at top of screen
	void displayWinner(int player);

    Ogre::Root *mRoot;				//root of Ogre graphics library
    Ogre::Camera* mCamera;			//camera to see
    Ogre::SceneManager* mSceneMgr;	//contains nodes & entities
    Ogre::RenderWindow* mWindow;	//window on screen

	Ogre::OverlaySystem *mOverlaySystem;	//overlay system to show GUI
	
    OgreBites::SdkTrayManager* mTrayMgr;	//manages locations for GUI
    OgreBites::SdkCameraMan* mCameraMan;	//camera movement
	std::vector<OgreBites::Button*> vecMenuButtons;	//GUI menu buttons
	std::vector<OgreBites::Label*> vecPlayerLabels;	//GUI player labels
	std::vector<OgreBites::Label*> vecWinnerLabels;	//GUI winner labels
   
	OgreBites::InputContext mInputContext;	//handler for inputs
	bool mCursorMode;	//flag to control mouse or camera
	
    bool mShutDown;	//flag to control if window is closing

    //OIS Input devices
    OIS::InputManager* mInputManager;	//manges the inputs
    OIS::Mouse*    mMouse;				//keyboard controller
    OIS::Keyboard* mKeyboard;			//mouse controller

	//Bullet Physics Variables
	Physics* physicsEngine;
	btCollisionShape* shapeTable;	//shape of the game board
	btCollisionShape* shapeGround;	//shape of the ground
	btCollisionShape* shapeStone;	//shape of the stones
	btRigidBody* rigidTable;		//physics object of game board
	btRigidBody* rigidGround;		//physics object of ground
	std::vector<btRigidBody*> vecRigidStones;		//physics objects of the stones
	btMotionState* motionTable;		//motion data of the game board
	btMotionState* motionGround;	//motion data of the ground
	std::vector<btMotionState*> vecMotionStones;	//motion data of the stones

	std::vector<Ogre::Entity*> vecEntityStones;		//graphics of the stones
	std::vector<Ogre::SceneNode*> vecNodeStones;	//nodes of the stones (position, orientation)

	GomokuBoard gBoard;		//game board data
	AIPlayer playerAI;		//AI player 1
	AIPlayer playerAI2;		//AI player 2

	Ogre::Vector3 mPickCoords;	//real mouse lookat coordinates
	int mBoardX;	//x mouse coordinate on board
	int mBoardY;	//y mouse coordinate on board
	bool mOnBoard;	//if currently looking at the board
	int mMenuState;	//controls which menu to show
	bool bGameOver;	//if game won / lost
	bool bPhysicsApplied;	//if the board has been flipped
	bool bGameStart; //if board is ready to play
	bool bHasFallen;  //if the board has started falling
	bool bGameVSAI;	//if game versus AI
	bool bGameAIVAI; //if 2 computers playing
	int mCurrentPlayer;	//player's turn
	int mGameWinner; //winner of the game
	float mAITurnTimer;	//timer; sets when the AI v AI players can make a move
};