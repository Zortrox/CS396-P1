/*
	GomokuGame.cpp
	Sets up the game, input, display, and resources

	@author Matthew Clark
	@date 09/02/2016
*/

#include "GomokuGame.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

GomokuGame::GomokuGame()
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mTrayMgr(0),
    mCameraMan(0),
	mCursorMode(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	mOverlaySystem(0)
{
	
}

GomokuGame::~GomokuGame(void)
{
	if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;
	if (mOverlaySystem) delete mOverlaySystem;

    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

bool GomokuGame::configure(void)
{
	//load config or display dialog if no file exists
    if(mRoot->showConfigDialog()) //mRoot->restoreConfig()
    {
        //create the game window
        mWindow = mRoot->initialise(true, "SANJIGEN ROBOTTO GOMOKUNARABE");
        return true;
    }
    else
    {
		//game not loaded (no file or cancelled config)
        return false;
    }
}

void GomokuGame::chooseSceneManager(void)
{
    //load the scene manager
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
	//load the GUI object
    mOverlaySystem = new Ogre::OverlaySystem();
	//add rendering listener to scene manager
    mSceneMgr->addRenderQueueListener(mOverlaySystem);
}

void GomokuGame::createCamera(void)
{
	//creates and sets the position of the camera
	mCamera = mSceneMgr->createCamera("PlayerCam");
	mCamera->setPosition(Ogre::Vector3(0, 4.5, 2));
	mCamera->setDirection(Ogre::Vector3(0, -0.3f, -1));
	mCamera->setNearClipDistance(0.01f);	//objects closer than this won't be rendered

	mCameraMan = new OgreBites::SdkCameraMan(mCamera); //Create camera controller
}

void GomokuGame::createFrameListener(void)
{
    Ogre::LogManager::getSingletonPtr()->logMessage("*** Initializing OIS ***");
    OIS::ParamList pl;
    size_t windowHnd = 0;
    std::ostringstream windowHndStr;

    mWindow->getCustomAttribute("WINDOW", &windowHnd);
    windowHndStr << windowHnd;
    pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

    mInputManager = OIS::InputManager::createInputSystem( pl );
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject( OIS::OISKeyboard, true ));
    mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject( OIS::OISMouse, true ));
	mInputContext.mKeyboard = mKeyboard;
    mInputContext.mMouse = mMouse;
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //set initial mouse clipping size
    windowResized(mWindow);

    //register as a window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

	//create default GUI objects
	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
    mTrayMgr->hideCursor();

	//create menu buttons
	vecMenuButtons.resize(menuButtons::COUNT);
	vecMenuButtons[menuButtons::B_NEW] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNew", "New Game");
	vecMenuButtons[menuButtons::B_QUIT] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonQuit", "Quit");
	vecMenuButtons[menuButtons::B_RESUME] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonResume", "Resume");
	vecMenuButtons[menuButtons::B_VSAI] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNewAI", "vs AI");
	vecMenuButtons[menuButtons::B_VSHUM] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNewHum", "vs Player");
	vecMenuButtons[menuButtons::B_AIVAI] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNewAIvAI", "AI vs AI");
	setMenu(menuState::NEW_GAME);

	//create top player labels
	vecPlayerLabels.resize(gameWinners::COUNT);
	vecPlayerLabels[gamePlayers::P_BLACK] = 
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelPlayer1", "Player 1 : (BLACK)", 200);
	vecPlayerLabels[gamePlayers::P_WHITE] = 
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelPlayer2", "Player 2 : (WHITE)", 200);
	mTrayMgr->removeWidgetFromTray(vecPlayerLabels[gamePlayers::P_WHITE]);
	vecPlayerLabels[gamePlayers::P_WHITE]->hide();

	//create winner labels
	vecWinnerLabels.resize(gameWinners::COUNT);
	vecWinnerLabels[gameWinners::WIN_BLACK] =
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelWinnerBlack", "Winner: Player 1 (BLACK)!", 250);
	vecWinnerLabels[gameWinners::WIN_WHITE] = 
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelWinnerWhite", "Winner: Player 2 (WHITE)!", 250);
	vecWinnerLabels[gameWinners::WIN_TIE] = 
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelWinnerTie", "TIE!  NO WINNER!", 250);
	mTrayMgr->removeWidgetFromTray(vecWinnerLabels[gameWinners::WIN_BLACK]);
	mTrayMgr->removeWidgetFromTray(vecWinnerLabels[gameWinners::WIN_WHITE]);
	mTrayMgr->removeWidgetFromTray(vecWinnerLabels[gameWinners::WIN_TIE]);
	vecWinnerLabels[gameWinners::WIN_BLACK]->hide();
	vecWinnerLabels[gameWinners::WIN_WHITE]->hide();
	vecWinnerLabels[gameWinners::WIN_TIE]->hide();

	//add the frame listener
    mRoot->addFrameListener(this);
}

void GomokuGame::createScene(void) {
	//robot graphics
	Ogre::Entity* entRobot = mSceneMgr->createEntity("Ninja", "robot.mesh");
	entRobot->setCastShadows(true);
	Ogre::SceneNode* nodeRobot = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, -3));
	nodeRobot->attachObject(entRobot);
	nodeRobot->setScale(Ogre::Vector3(.05f, .05f, .05f));
	nodeRobot->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(-3.14f / 2.0f));

	//stone physics
	shapeStone = new btCylinderShape(btVector3(0.075f, 0.025f, 0.075f));

	//game board graphics
	Ogre::Entity* entTable = mSceneMgr->createEntity("GomokuTable", "GomokuTable.mesh");
	Ogre::SceneNode* nodeTable = mSceneMgr->getRootSceneNode()->createChildSceneNode("GomokuTable", Ogre::Vector3(0, 10, 0));
	nodeTable->attachObject(entTable);
	entTable->setCastShadows(true);
	nodeTable->setScale(Ogre::Vector3(2, 2, 2));
	//game board physics
	btTransform transformTable;
	transformTable.setIdentity();
	btScalar mass = 10.0f;
	btVector3 localInertia(0, 0, 0);
	shapeTable = new btBoxShape(btVector3(2.0f, 0.18f, 2.0f));
	transformTable.setOrigin(btVector3(0, -10, 0));
	shapeTable->calculateLocalInertia(mass, localInertia);
	motionTable = new btDefaultMotionState(transformTable);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionTable, shapeTable, localInertia);
	rigidTable = new btRigidBody(rbInfo);
	rigidTable->setRestitution(1);
	rigidTable->setFriction(2);
	rigidTable->setUserPointer(nodeTable);
	physicsEngine->getDynamicsWorld()->addRigidBody(rigidTable);
	//stop physics until new game started
	rigidTable->setActivationState(0);

	//ground graphics
	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane, 20, 20, 10, 10, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
	Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
	Ogre::SceneNode* nodeGround = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nodeGround->attachObject(entGround);
	entGround->setMaterialName("Examples/Rockwall");
	entGround->setCastShadows(false);
	//ground physics
	btTransform transformGround;
	transformGround.setIdentity();
	btScalar massGround = 0;	//0 means it can't move
	btVector3 localGroundInertia(0, 0, 0);
	shapeGround = new btBoxShape(btVector3(btScalar(10), btScalar(1), btScalar(10)));
	transformGround.setOrigin(btVector3(0, -1, 0));
	motionGround = new btDefaultMotionState(transformGround);
	shapeGround->calculateLocalInertia(massGround, localGroundInertia);
	btRigidBody::btRigidBodyConstructionInfo groundRBInfo(massGround, motionGround, shapeGround, localGroundInertia);
	rigidGround = new btRigidBody(groundRBInfo);
	rigidGround->setFriction(5);
	physicsEngine->getDynamicsWorld()->addRigidBody(rigidGround);

	//selector stone graphics
	Ogre::SceneNode* pickNode = nodeTable->createChildSceneNode("PickNode");
	Ogre::Entity* entStone = mSceneMgr->createEntity("PickStone", "GomokuStonePick.mesh");
	pickNode->attachObject(entStone);
	pickNode->setScale(0.081f, 0.081f, 0.081f);

	//LIGHTS --------------------------------------------------------------------------
	//directional light (like from sun)
	Ogre::Light* directionalLight = mSceneMgr->createLight("directionalLight");
	directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
	directionalLight->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setDirection(Ogre::Vector3(0, -1, -0.3f));

	//ambient light (global illumination)
	mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f, .5f, .5f));
	//mSceneMgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));

	//Spotlight (more "dark and creepy" atmosphere
	//Ogre::Light* spotlLight = mSceneMgr->createLight("spotLight");
	//spotlLight->setType(Ogre::Light::LT_SPOTLIGHT);
	//spotlLight->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	//spotlLight->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	//spotlLight->setDirection(Ogre::Vector3(0, -1, 0));
	//spotlLight->setPosition(Ogre::Vector3(0, 50, 0));

	//shadow technique; sharp shadows
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

	//fog
	//mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	//mSceneMgr->setFog(Ogre::FOG_EXP2, Ogre::ColourValue(0, 0, 0), 0.07f);

	//create listing of all skyboxes
	vecSkyboxes.push_back("MPBloodSkyBox");
	vecSkyboxes.push_back("MPMandarisSkyBox");
	vecSkyboxes.push_back("SkyBasic01SkyBox");
	vecSkyboxes.push_back("SkyBasic02SkyBox");
	vecSkyboxes.push_back("SkyBasic03SkyBox");
	vecSkyboxes.push_back("SkyBasic04SkyBox");
	vecSkyboxes.push_back("SkyBasic05SkyBox");
	vecSkyboxes.push_back("SkyBasic06SkyBox");
	vecSkyboxes.push_back("SkyBasic07SkyBox");
	vecSkyboxes.push_back("SkyBasic08SkyBox");
	vecSkyboxes.push_back("SkyBasic09SkyBox");
	vecSkyboxes.push_back("SkyBasic10SkyBox");
	vecSkyboxes.push_back("SkyBasic11SkyBox");
	vecSkyboxes.push_back("SkyBasic12SkyBox");
	vecSkyboxes.push_back("SkyBasic13SkyBox");
	vecSkyboxes.push_back("SkyBasic14SkyBox");
	vecSkyboxes.push_back("SkyBasic15SkyBox");
	vecSkyboxes.push_back("SkyBasic16SkyBox");
	vecSkyboxes.push_back("SkyBasic17SkyBox");
	vecSkyboxes.push_back("SkyBasic18SkyBox");
	vecSkyboxes.push_back("SkyBasic19SkyBox");
	vecSkyboxes.push_back("SkyBasic20SkyBox");
	vecSkyboxes.push_back("SkyBasic21SkyBox");
	vecSkyboxes.push_back("SkyBasic22SkyBox");
	vecSkyboxes.push_back("SkyBasic23SkyBox");
	vecSkyboxes.push_back("SkyBasic24SkyBox");
	vecSkyboxes.push_back("SkyBasic25SkyBox");
	vecSkyboxes.push_back("SkyBasic26SkyBox");
	vecSkyboxes.push_back("SkyBasic27SkyBox");
	vecSkyboxes.push_back("SkyBasic28SkyBox");
	vecSkyboxes.push_back("SkyBasic29SkyBox");
	vecSkyboxes.push_back("SkyBasic30SkyBox");

	//preload skyboxes
	//for (size_t skies = 0; skies < vecSkyboxes.size(); skies++) {
	//	Ogre::MaterialManager::getSingleton().load(vecSkyboxes[skies], "skyboxes");
	//}

	//set skybox
	mCurrentSkybox = rand() % vecSkyboxes.size();
	mSkyboxDist = 100;
	mSceneMgr->setSkyBox(true, vecSkyboxes[mCurrentSkybox], mSkyboxDist);
}

void GomokuGame::createViewports(void)
{
	//create one viewport; size of the entire window
	Ogre::Viewport* vp = mWindow->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	//change the aspect ratio to match viewport size
	mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}

void GomokuGame::setupResources(void)
{
	//add resource locations
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./assets/materials/scripts", "FileSystem", "materials");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./assets/materials/textures", "FileSystem", "textures");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./assets/materials/textures/skyboxes", "FileSystem", "skyboxes");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./assets/models", "FileSystem", "models");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./assets/packs/SdkTrays.zip", "Zip", "SdkTrays");

	//load all resources at locations
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void GomokuGame::go(void)
{
	//quit if the game didn't set up correctly
    if (!setup())
        return;

	//start drawing otherwise
    mRoot->startRendering();
}

bool GomokuGame::setup(void)
{
	//Ogre base graphics engine
	mRoot = new Ogre::Root("", "ogre.cfg", "LowLevelOgre.log");

	//plugins required for game
	Ogre::StringVector plugins_toLoad;
	plugins_toLoad.push_back("RenderSystem_GL");	//openGL

	//load the plugins (based on debug/release build)
	for (Ogre::StringVector::iterator j = plugins_toLoad.begin(); j != plugins_toLoad.end(); j++)
	{
#ifdef _DEBUG
		mRoot->loadPlugin(*j + Ogre::String("_d"));
#else
		mRoot->loadPlugin(*j);
#endif;
	}

	//initialize physics
	physicsEngine = new Physics();

	//initialize game variables
	srand(time(NULL));
	mBoardX = 0;
	mBoardY = 0;
	mOnBoard = false;
	mMenuState = menuState::CLOSED;
	bGameOver = true;
	bPhysicsApplied = false;
	bGameStart = false;
	bHasFallen = false;
	bGameVSAI = false;
	mCurrentPlayer = gamePlayers::P_BLACK;
	mGameWinner = gameWinners::COUNT;
	playerAI.init(&gBoard);
	playerAI2.init(&gBoard);
	mAITurnTimer = 0.0f;

	//stop if graphics configuration is bad
    if (!configure()) return false;

	//setup ogre graphics
    chooseSceneManager();
    createCamera();
    createViewports();

    //set default mipmap level
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

	//setup more graphics
	setupResources();
    createScene();

	//start frame listener
    createFrameListener();

    return true;
};

bool GomokuGame::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
	//stop rendering if window is closed or game is shutdown
    if(mWindow->isClosed() || mShutDown)
        return false;
	
	//step physics
	if (physicsEngine != NULL) {
		physicsEngine->getDynamicsWorld()->stepSimulation(evt.timeSinceLastFrame);

		//get all physics activated objects (objects where physics is now applying)
		btCollisionObjectArray arrObj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray();

		//loop through objects
		for (int i = 0; i < arrObj.size(); i++) {
			btCollisionObject* obj = arrObj[i];
			btRigidBody* body = btRigidBody::upcast(obj);	//cast to rigidbody

			//if object is in motion, transform node (graphics) to physics position/orientation
			if (body && body->getMotionState()) {
				btTransform trans;
				body->getMotionState()->getWorldTransform(trans);

				void* userPointer = body->getUserPointer();
				if (userPointer) {
					btQuaternion orientation = trans.getRotation();
					Ogre::SceneNode *sceneNode = static_cast<Ogre::SceneNode*>(userPointer);
					sceneNode->setPosition(Ogre::Vector3(trans.getOrigin().getX(), trans.getOrigin().getY(), trans.getOrigin().getZ()));
					sceneNode->setOrientation(Ogre::Quaternion(orientation.getW(), orientation.getX(), orientation.getY(), orientation.getZ()));
				}
			}
		}
	}

	//start next turn when board has fallen to the ground & is ready
	if (!bGameOver && rigidTable->getLinearVelocity().length() > 0.1f) {
		bHasFallen = true;
	}
	if (!bGameStart && !bGameOver && bHasFallen && rigidTable->getLinearVelocity().length() < 0.001f) {
		bGameStart = true;
		nextTurn();
	}

	//if game is AI v AI, delay each AI's moves
	//otherwise the game is over almost instantly
	float timeWait = 0.05f;	//seconds
	if (bGameStart && bGameAIVAI && !bGameOver) {
		mAITurnTimer += evt.timeSinceLastFrame;
		if (mAITurnTimer >= timeWait) {
			mAITurnTimer = 0.0f;
			nextTurn();
		}
	}

	//get any input from the input buffer
	//this is basically the logic loop
	//and human player turn
	mInputContext.capture();

	//set the tile selector when to be visible
	bool bVisible = true;
	//hidden if not looking at the board, looking at a non-empty tile, when the game is over, and when the mode is AI v AI
	if (!mOnBoard || !gBoard.emptyTile(mBoardX, mBoardY) || bGameOver || bGameAIVAI) bVisible = false;
	mSceneMgr->getSceneNode("PickNode")->setVisible(bVisible);

	//render the GUI
    mTrayMgr->frameRenderingQueued(evt);
	//render the camera
    mCameraMan->frameRenderingQueued(evt);

    return true;
}
//-------------------------------------------------------------------------------------
bool GomokuGame::keyPressed( const OIS::KeyEvent &arg )
{
	//"PrtScn" or "SysRq" button
	if (arg.key == OIS::KC_SYSRQ)
	{
		// take a screenshot
		mWindow->writeContentsToTimestampedFile("screenshot-", ".jpg");
	}
	else if (arg.key == OIS::KC_ESCAPE)
	{
		//open or close the menu
		if (mMenuState == menuState::MAIN) {
			setMenu(menuState::CLOSED);
		}
		else if (mMenuState == menuState::CLOSED) {
			setMenu(menuState::MAIN);
		}
		else if (mMenuState == menuState::NEW_GAME) {
			setMenu(menuState::MAIN);
		}
	}

	//mCameraMan->injectKeyDown(arg);
	return true;
}

bool GomokuGame::keyReleased(const OIS::KeyEvent &arg)
{
	//mCameraMan->injectKeyUp(arg);
	return true;
}

bool GomokuGame::mouseMoved(const OIS::MouseEvent &arg)
{
	//get the mouse look coordinates @ board height
	mPickCoords = getGameLookCoords();
	int size = gBoard.getBoardSize();
	//normalize and align stone to board grid size
	float pX = floor((mPickCoords.x + .6501f) / 1.203f * (size - 1));
	float pY = mPickCoords.y;
	float pZ = floor((mPickCoords.z + .6501f) / 1.203f * (size - 1));
	mBoardX = pX;
	mBoardY = pZ;

	//gives some buffer to game board edges
	int bufferSize = 5;
	if (mBoardX < 0 && mBoardX >= -bufferSize) mBoardX = 0;
	if (mBoardX > (size - 1) && mBoardX <= (size - 1) + bufferSize) mBoardX = size - 1;
	if (mBoardY < 0 && mBoardY >= -bufferSize) mBoardY = 0;
	if (mBoardY > (size - 1) && mBoardY <= (size - 1) + bufferSize) mBoardY = size - 1;

	//hide the cursor if not looking at the board
	if (mBoardX < 0 || mBoardX > size - 1 || mBoardY < 0 || mBoardY > size - 1) {
		mOnBoard = false;
	}
	else {
		mOnBoard = true;
	}

	//get board grid indices from the grid-aligned mouse coordinates
	mPickCoords = Ogre::Vector3(mBoardX * 0.086f - 0.61f, pY, mBoardY * 0.086f - 0.61f);

	//move selector stone to grid index
	Ogre::SceneNode* pickNode = mSceneMgr->getSceneNode("PickNode");
	pickNode->setPosition(mPickCoords);

	//move the mouse in the menu if menu is being shown, otherwise move the camera
	if (mCursorMode && mTrayMgr->injectMouseMove(arg)) return true;
	else if (!mCursorMode) mCameraMan->injectMouseMove(arg);
	return true;
}

bool GomokuGame::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	//try to add stone to board on left click
	if (id == OIS::MB_Left) {
		if (mOnBoard && !mCursorMode && !bGameOver && !bGameAIVAI && bGameStart) {
			//add stone if able to
			addStoneToBoard(mBoardX, mBoardY);
		}
	}

	//if cursor mode, access GUI functions
	//otherwise allow camera to get mouse presses
    if (mCursorMode && mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GomokuGame::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	//if cursor mode, access GUI functions
	//otherwise allow camera to get mouse releases
    if (mCursorMode && mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

void GomokuGame::buttonHit(OgreBites::Button * button)
{
	//close the game if "Quit" is clicked
	if (button->getName() == "buttonQuit") {
		mShutDown = true;
	}
	//close the menu
	else if (button->getName() == "buttonResume") {
		setMenu(menuState::CLOSED);
	}
	//show new game type selector
	else if (button->getName() == "buttonNew") {
		bGameOver = true;
		rigidTable->activate();
		setStonePhysics();
		setMenu(menuState::NEW_GAME);
	}
	//start new P v AI game
	else if (button->getName() == "buttonNewAI") {
		bGameVSAI = true;
		bGameAIVAI = false;
		resetGame();
		setMenu(menuState::CLOSED);
	}
	//start new P v P game
	else if (button->getName() == "buttonNewHum") {
		bGameVSAI = false;
		bGameAIVAI = false;
		resetGame();
		setMenu(menuState::CLOSED);
	}
	//start new AI v AI game
	else if (button->getName() == "buttonNewAIvAI") {
		bGameVSAI = false;
		bGameAIVAI = true;
		resetGame();
		setMenu(menuState::CLOSED);
	}
}

void GomokuGame::setMenu(int state) {
	//clear all menu items to add new	
	removeAllMenuItems(mTrayMgr->getTrayContainer(OgreBites::TL_CENTER));

	//hide and show menu buttons based on the state
	switch (state) {
	case menuState::CLOSED:
		mCursorMode = false;
		mTrayMgr->hideCursor();
		mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->hide();
		break;
	case menuState::MAIN:
		if (mMenuState == menuState::CLOSED) {	//if menu was previously closed
			mCursorMode = true;
			mTrayMgr->showCursor();
			mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->show();
		}
		mTrayMgr->moveWidgetToTray(vecMenuButtons[menuButtons::B_RESUME]->getName(), OgreBites::TL_CENTER);
		vecMenuButtons[menuButtons::B_RESUME]->show();
		mTrayMgr->moveWidgetToTray(vecMenuButtons[menuButtons::B_NEW]->getName(), OgreBites::TL_CENTER);
		vecMenuButtons[menuButtons::B_NEW]->show();
		mTrayMgr->moveWidgetToTray(vecMenuButtons[menuButtons::B_QUIT]->getName(), OgreBites::TL_CENTER);
		vecMenuButtons[menuButtons::B_QUIT]->show();
		break;
	case menuState::NEW_GAME:
		if (mMenuState == menuState::CLOSED) {	//if menu was previously closed
			mCursorMode = true;
			mTrayMgr->showCursor();
			mTrayMgr->getTrayContainer(OgreBites::TL_CENTER)->show();
		}
		mTrayMgr->moveWidgetToTray(vecMenuButtons[menuButtons::B_VSAI]->getName(), OgreBites::TL_CENTER);
		vecMenuButtons[menuButtons::B_VSAI]->show();
		mTrayMgr->moveWidgetToTray(vecMenuButtons[menuButtons::B_VSHUM]->getName(), OgreBites::TL_CENTER);
		vecMenuButtons[menuButtons::B_VSHUM]->show();
		mTrayMgr->moveWidgetToTray(vecMenuButtons[menuButtons::B_AIVAI]->getName(), OgreBites::TL_CENTER);
		vecMenuButtons[menuButtons::B_AIVAI]->show();
		break;
	}

	//set current menu state
	mMenuState = state;
}

void GomokuGame::setPlayerLabel(bool reset)
{
	//remove current player label
	mTrayMgr->removeWidgetFromTray(vecPlayerLabels[mCurrentPlayer]);
	vecPlayerLabels[mCurrentPlayer]->hide();

	//select next player
	if (mCurrentPlayer == gamePlayers::P_WHITE || reset) {
		mCurrentPlayer = gamePlayers::P_BLACK;
	}
	else if (mCurrentPlayer == gamePlayers::P_BLACK) {
		mCurrentPlayer = gamePlayers::P_WHITE;
	}

	//add next player label
	mTrayMgr->moveWidgetToTray(vecPlayerLabels[mCurrentPlayer], OgreBites::TL_TOP);
	vecPlayerLabels[mCurrentPlayer]->show();
}

void GomokuGame::windowResized(Ogre::RenderWindow* rw)
{
	//get widow dimensions
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

	//constrain mouse
    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

void GomokuGame::windowClosed(Ogre::RenderWindow* rw)
{
    //only close for window that created OIS
    if( rw == mWindow )
    {
		//clear out input object pointers
        if( mInputManager )
        {
            mInputManager->destroyInputObject( mMouse );
            mInputManager->destroyInputObject( mKeyboard );

            OIS::InputManager::destroyInputSystem(mInputManager);
            mInputManager = 0;
        }
    }
}

Ogre::Vector3 GomokuGame::getGameLookCoords() {
	//get the camera direction and multiply it by the height to get the x and z coordinates
	Ogre::Vector3 dir = mCamera->getDirection();
	Ogre::Vector3 pos = mCamera->getPosition();
	float baseDist = Ogre::Vector3::NEGATIVE_UNIT_Y.distance(dir) * pos.y;

	//offset by board height and camera position
	Ogre::Vector3 coords = Ogre::Vector3(baseDist * dir.x + pos.x, 0.1f, baseDist * dir.z + pos.z / 2);

	return coords;
}

void GomokuGame::setStonePhysics() {
	//flip board if physics not already applied
	if (!bPhysicsApplied) {
		std::vector<GameTile> vecStones = gBoard.getAllStones();

		for (size_t i = 0; i < vecStones.size(); i++) {
			//remove all nodes and entities from the game board
			Ogre::SceneNode* nodeStone = mSceneMgr->getSceneNode(vecStones[i].nodeName);
			Ogre::Vector3 pos = nodeStone->_getDerivedPosition();
			mSceneMgr->destroyEntity(vecStones[i].entName);
			mSceneMgr->destroySceneNode(vecStones[i].nodeName);

			//create new nodes and entities at the same location (not sub-nodes)
			Ogre::Entity* entStone;
			if (vecStones[i].color == stoneColor::BLACK) {
				entStone = mSceneMgr->createEntity(vecStones[i].entName, "GomokuStoneBlack.mesh");
			}
			else {
				entStone = mSceneMgr->createEntity(vecStones[i].entName, "GomokuStoneWhite.mesh");
			}
			nodeStone = mSceneMgr->getRootSceneNode()->createChildSceneNode(vecStones[i].nodeName, pos);
			nodeStone->attachObject(entStone);
			entStone->setCastShadows(true);
			nodeStone->setScale(0.15f, 0.15f, 0.15f);
			vecEntityStones.push_back(entStone);
			vecNodeStones.push_back(nodeStone);

			//add physics objects to each one now
			btTransform startTransform;
			startTransform.setIdentity();
			btScalar mass = .02f;
			btVector3 localInertia(0, 0, 0);
			startTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
			shapeStone->calculateLocalInertia(mass, localInertia);
			btDefaultMotionState* motionStone = new btDefaultMotionState(startTransform);
			vecMotionStones.push_back(motionStone);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionStone, shapeStone, localInertia);
			btRigidBody *stoneBody = new btRigidBody(rbInfo);
			vecRigidStones.push_back(stoneBody);
			stoneBody->setRestitution(0.1f);
			stoneBody->setUserPointer(nodeStone);
			stoneBody->setFriction(1.5);
			physicsEngine->getDynamicsWorld()->addRigidBody(stoneBody);
		}

		//"flip" the table based on who won
		int flipDirection = 1;
		if ((bGameVSAI || bGameAIVAI) && playerAI.getPlayerNum() != mGameWinner && bGameOver) flipDirection = -1;
		rigidTable->applyImpulse(btVector3(0, 30, 50 * flipDirection), btVector3(0, 0, 10 * flipDirection));
	}

	//don't apply physics again
	bPhysicsApplied = true;
}

bool GomokuGame::addStoneToBoard(int xGrid, int yGrid)
{
	//create stone names
	std::string strNum = std::to_string(xGrid) + "_" + std::to_string(yGrid);
	std::string strEntity = "entStone_" + strNum;
	std::string strNode = "nodeStone_" + strNum;

	//convert player to stone color
	int color;
	switch (mCurrentPlayer) {
	case gamePlayers::P_BLACK:
		color = stoneColor::BLACK;
		break;
	case gamePlayers::P_WHITE:
		color = stoneColor::WHITE;
		break;
	}

	//place stone at location if not already one there
	if (gBoard.addStone(xGrid, yGrid, color, strEntity, strNode)) {
		addStoneGraphics(strEntity, strNode, xGrid, yGrid);

		//end game if won, otherwise switch players
		if (gBoard.gameWon() && !bGameOver) {
			//if game was won (not just reset)
			bGameOver = true;
			mGameWinner = mCurrentPlayer;
			displayWinner(mGameWinner);
		}
		else if (gBoard.boardFilled()) {
			//tie occurred
			bGameOver = true;
			mGameWinner = gameWinners::WIN_TIE;
			displayWinner(mGameWinner);
		}
		else {
			//change label at top of screen
			setPlayerLabel();

			//cycle next turn
			if (!bGameAIVAI) {
				nextTurn();
			}
		}

		return true;
	}
	else return false;
}

void GomokuGame::addStoneGraphics(std::string strEntity, std::string strNode, int xGrid, int yGrid)
{
	//add stone to board with entity and node
	Ogre::Entity* entStone;
	if (mCurrentPlayer == gamePlayers::P_BLACK) {
		entStone = mSceneMgr->createEntity(strEntity, "GomokuStoneBlack.mesh");
	}
	else if (mCurrentPlayer == gamePlayers::P_WHITE) {
		entStone = mSceneMgr->createEntity(strEntity, "GomokuStoneWhite.mesh");
	}
	
	Ogre::SceneNode* tableNode = mSceneMgr->getSceneNode("GomokuTable");
	Ogre::SceneNode* stoneNode = tableNode->createChildSceneNode(strNode,
		(Ogre::Vector3((float)xGrid * 0.086f - 0.61f, 0.1f, (float)yGrid * 0.086f - 0.61f)));
	stoneNode->attachObject(entStone);
	stoneNode->setScale(0.081f, 0.081f, 0.081f);
}

void GomokuGame::nextTurn()
{
	//execute AI moves
	//first AI
	if ((bGameVSAI || bGameAIVAI) && mCurrentPlayer == playerAI.getPlayerNum() && bGameStart) {
		TilePos* move;
		//keep looping to find a valid stone spot (shouldn't loop more than once)
		do {
			move = playerAI.getNextMove();
		} while (!addStoneToBoard(move->xGrid, move->yGrid));
	}
	//second AI (if AI v AI)
	else if (bGameAIVAI && mCurrentPlayer == playerAI2.getPlayerNum() && bGameStart) {
		TilePos* move;
		//keep looping to find a valid stone spot (shouldn't loop more than once)
		do {
			move = playerAI2.getNextMove();
		} while (!addStoneToBoard(move->xGrid, move->yGrid));
	}
}

void GomokuGame::resetGame() {
	//remove stone physics, entities, and nodes
	for (size_t i = 0; i < vecRigidStones.size(); i++) {
		physicsEngine->getDynamicsWorld()->removeRigidBody(vecRigidStones[i]);
		delete vecRigidStones[i];
		delete vecMotionStones[i];

		mSceneMgr->destroyEntity(vecEntityStones[i]);
		mSceneMgr->destroySceneNode(vecNodeStones[i]);
	}
	vecRigidStones.clear();
	vecMotionStones.clear();
	vecEntityStones.clear();
	vecNodeStones.clear();

	//clear board grid
	gBoard.clearBoard();
	bGameOver = false;
	bPhysicsApplied = false;
	bGameStart = false;
	bHasFallen = false;

	//reset AI move timer
	mAITurnTimer = 0.0f;

	//clear AI weights
	setPlayerLabel(true);
	int playerAINum;
	if (bGameVSAI || bGameAIVAI) {
		playerAI.reset();
		
		//set new AI player number
		playerAINum = rand() % gamePlayers::COUNT;
		int newColor;
		switch (playerAINum) {
		case gamePlayers::P_BLACK:
			newColor = stoneColor::BLACK;
			break;
		case gamePlayers::P_WHITE:
			newColor = stoneColor::WHITE;
			break;
		}
		playerAI.setPlayerNum(playerAINum, newColor, playerAINum == gamePlayers::P_BLACK);

		//reset AI #2
		if (bGameAIVAI) {
			playerAI2.reset();

			//set new AI player number that isn't the other player's number
			//written to allow for multiple players in the future
			int playerAINum2;
			do {
				playerAINum2 = rand() % gamePlayers::COUNT;
			}
			while (playerAINum2 == playerAINum);

			int newColor;
			switch (playerAINum2) {
			case gamePlayers::P_BLACK:
				newColor = stoneColor::BLACK;
				break;
			case gamePlayers::P_WHITE:
				newColor = stoneColor::WHITE;
				break;
			}
			playerAI2.setPlayerNum(playerAINum2, newColor, playerAINum2 == gamePlayers::P_BLACK);
		}
	}
	//set player label back to black (player 1)
	setPlayerLabel(true);

	//new skybox
	mCurrentSkybox = rand() % vecSkyboxes.size();
	mSceneMgr->setSkyBox(true, vecSkyboxes[mCurrentSkybox], mSkyboxDist);

	//clear winner from screen & reset previous game winner (if not first time running the game)
	if (mGameWinner != gameWinners::COUNT) {
		mTrayMgr->removeWidgetFromTray(vecWinnerLabels[mGameWinner]);
		vecWinnerLabels[mGameWinner]->hide();
	}
	mGameWinner = gameWinners::COUNT;

	//set board back to starting position and reset its motion
	btTransform initialTransform;
	initialTransform.setOrigin(btVector3(0, 10, 0));
	initialTransform.setRotation(btQuaternion(0, 0, 0, 1));
	rigidTable->clearForces();
	rigidTable->setLinearVelocity(btVector3(0, 0, 0));
	rigidTable->setAngularVelocity(btVector3(0, 0, 0));
	rigidTable->setWorldTransform(initialTransform);
	motionTable->setWorldTransform(initialTransform);
	rigidTable->activate();
}

void GomokuGame::removeAllMenuItems(Ogre::OverlayContainer * menuContainer)
{
	//remove and hide each item from menu
	for (size_t i = 0; i < vecMenuButtons.size(); i++) {
		mTrayMgr->removeWidgetFromTray(vecMenuButtons[i]->getName());
		vecMenuButtons[i]->hide();
	}
}

void GomokuGame::displayWinner(int player)
{
	//show winner label
	mTrayMgr->moveWidgetToTray(vecWinnerLabels[player], OgreBites::TL_TOP);
	vecWinnerLabels[player]->show();
	
	//hide previous player turn label
	mTrayMgr->removeWidgetFromTray(vecPlayerLabels[mCurrentPlayer]);
	vecPlayerLabels[mCurrentPlayer]->hide();
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
{
	//main game object
	GomokuGame game;

	try {
		//try to start the game
		game.go();
	}
	catch (Ogre::Exception& e) {
		//usually DLL or resource pack missing
		MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}

	return 0;
}