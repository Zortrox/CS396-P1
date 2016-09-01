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

namespace menuState {
	enum menuState { CLOSED, MAIN, NEW_GAME };
}
namespace menuButtons {
	enum menuButtons { B_QUIT, B_NEW, B_RESUME, B_VSAI, B_VSHUM, COUNT };
}

class GomokuGame : public Ogre::FrameListener, public Ogre::WindowEventListener, public OIS::KeyListener, public OIS::MouseListener, OgreBites::SdkTrayListener
{
public:
	GomokuGame();
    virtual ~GomokuGame();

    virtual void go(void);

protected:
    virtual bool setup();
    virtual bool configure(void);
    virtual void chooseSceneManager(void);
    virtual void createCamera(void);
    virtual void createFrameListener(void);
	virtual void createScene(void);
    virtual void destroyScene(void);
    virtual void createViewports(void);
    virtual void setupResources(void);
    virtual void createResourceListener(void);
    virtual void loadResources(void);

    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );

	virtual void buttonHit(OgreBites::Button* button);
	virtual void setMenu(int state);
	virtual void setPlayerLabel(bool reset = false);

    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);

	virtual Ogre::Vector3 getGameLookCoords();
	virtual void setStonePhysics();
	virtual bool addStoneToBoard(int xGrid, int yGrid);
	virtual void addStoneGraphics(std::string strEntity, std::string strNode, int xGrid, int yGrid);
	virtual void nextTurn(bool reset = false);
	virtual void resetGame();
	virtual void removeAllMenuItems(Ogre::OverlayContainer* menuContainer);

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;

	Ogre::OverlaySystem *mOverlaySystem;

    OgreBites::SdkTrayManager* mTrayMgr;
    OgreBites::SdkCameraMan* mCameraMan;
    OgreBites::ParamsPanel* mDetailsPanel;
	std::vector<OgreBites::Button*> vecMenuButtons;
	OgreBites::Label* labelPlayer1;
	OgreBites::Label* labelPlayer2;
   
	OgreBites::InputContext mInputContext;
	
	bool mCursorMode;
	bool mCursorWasVisible;
    bool mShutDown;

    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;

	//Bullet Physics Variables
	Physics* physicsEngine;
	btCollisionShape* shapeTable;
	btCollisionShape* shapeGround;
	btCollisionShape* shapeStone;
	btRigidBody* rigidTable;
	btRigidBody* rigidGround;
	std::vector<btRigidBody*> vecRigidStones;
	btMotionState* motionTable;
	btMotionState* motionGround;
	std::vector<btMotionState*> vecMotionStones;

	std::vector<Ogre::Entity*> vecEntityStones;
	std::vector<Ogre::SceneNode*> vecNodeStones;

	GomokuBoard gBoard;
	AIPlayer playerAI;

	Ogre::Vector3 mPickCoords;	//real mouse lookat coordinates
	int mBoardX;	//x mouse coordinate on board
	int mBoardY;	//y mouse coordinate on board
	bool mOnBoard;	//if currently looking at the board
	int mMenuState;	//controls which menu to show
	bool bGameOver;	//if game won / lost
	bool bGameVSAI;	//if game versus AI
	int turnColor;	//turn stone color
};