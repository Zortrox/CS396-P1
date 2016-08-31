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

enum menuState { CLOSED, MAIN, NEW_GAME };
enum menuButtons { B_QUIT, B_NEW, B_RESUME, B_VSAI, B_VSHUM, TOTAL};

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
	virtual void GomokuGame::setMenu(int state);

    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);

	virtual void shootBox();
	virtual Ogre::Vector3 getGameLookCoords();
	virtual void setStonePhysics();
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

	int numBoxes;
	Ogre::Vector3 mPickCoords;
	GomokuBoard gBoard;
	int mBoardX;
	int mBoardY;
	bool mOnBoard;
	int mMenuState;
	bool bGameOver;
	bool bGameVSAI;
	bool playerTurn;
};