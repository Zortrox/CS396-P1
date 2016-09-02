#include "GomokuGame.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

GomokuGame::GomokuGame()
    : mRoot(0),
    mCamera(0),
    mSceneMgr(0),
    mWindow(0),
    mResourcesCfg(Ogre::StringUtil::BLANK),
    mPluginsCfg(Ogre::StringUtil::BLANK),
    mTrayMgr(0),
    mCameraMan(0),
    mDetailsPanel(0),
	mCursorMode(false),
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	mOverlaySystem(0)
{
	
}

//-------------------------------------------------------------------------------------
GomokuGame::~GomokuGame(void)
{
	if (mTrayMgr) delete mTrayMgr;
    if (mCameraMan) delete mCameraMan;
	if (mOverlaySystem) delete mOverlaySystem;

    Ogre::WindowEventUtilities::removeWindowEventListener(mWindow, this);
    windowClosed(mWindow);
    delete mRoot;
}

//-------------------------------------------------------------------------------------
bool GomokuGame::configure(void)
{
    // Show the configuration dialog and initialise the system
    // You can skip this and use root.restoreConfig() to load configuration
    // settings if you were sure there are valid ones saved in ogre.cfg
    if(mRoot->showConfigDialog())
    {
        // If returned true, user clicked OK so initialise
        // Here we choose to let the system create a default rendering window by passing 'true'
        mWindow = mRoot->initialise(true, "SANJIGEN ROBOTTO GOMOKUNARABE");

        return true;
    }
    else
    {
        return false;
    }
}
//-------------------------------------------------------------------------------------
void GomokuGame::chooseSceneManager(void)
{
    // Get the SceneManager, in this case a generic one
    mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
	//Fix for 1.9
    mOverlaySystem = new Ogre::OverlaySystem();
    mSceneMgr->addRenderQueueListener(mOverlaySystem);
}
//-------------------------------------------------------------------------------------
void GomokuGame::createCamera(void)
{
	mCamera = mSceneMgr->createCamera("PlayerCam");
	mCamera->setPosition(Ogre::Vector3(0, 4.5, 2));
	mCamera->lookAt(Ogre::Vector3(0, 0, 0));
	mCamera->setNearClipDistance(0.01f);

	mCameraMan = new OgreBites::SdkCameraMan(mCamera); //Create camera controller
}
//-------------------------------------------------------------------------------------
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

	//Fix for 1.9
	mInputContext.mKeyboard = mKeyboard;
    mInputContext.mMouse = mMouse;

    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);

    //Set initial mouse clipping size
    windowResized(mWindow);

    //Register as a Window listener
    Ogre::WindowEventUtilities::addWindowEventListener(mWindow, this);

	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->hideCursor();

    // create a params panel for displaying sample details
    Ogre::StringVector items;
    items.push_back("cam.pX");
    items.push_back("cam.pY");
    items.push_back("cam.pZ");
    items.push_back("");
    items.push_back("cam.oW");
    items.push_back("cam.oX");
    items.push_back("cam.oY");
    items.push_back("cam.oZ");
    items.push_back("");
    items.push_back("Filtering");
    items.push_back("Poly Mode");
	items.push_back("");
	items.push_back("X-Lookat");
	items.push_back("Z-Lookat");

    mDetailsPanel = mTrayMgr->createParamsPanel(OgreBites::TL_NONE, "DetailsPanel", 200, items);
    mDetailsPanel->setParamValue(9, "Bilinear");
    mDetailsPanel->setParamValue(10, "Solid");
    mDetailsPanel->hide();

	vecMenuButtons.resize(menuButtons::COUNT);
	vecMenuButtons[menuButtons::B_NEW] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNew", "New Game");
	vecMenuButtons[menuButtons::B_QUIT] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonQuit", "Quit");
	vecMenuButtons[menuButtons::B_RESUME] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonResume", "Resume");
	vecMenuButtons[menuButtons::B_VSAI] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNewAI", "vs AI");
	vecMenuButtons[menuButtons::B_VSHUM] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNewHum", "vs Player");
	vecMenuButtons[menuButtons::B_AIVAI] = mTrayMgr->createButton(OgreBites::TL_CENTER, "buttonNewAIvAI", "AI vs AI");
	setMenu(menuState::NEW_GAME);

	vecPlayerLabels.resize(gameWinners::COUNT);
	vecPlayerLabels[gamePlayers::P_BLACK] = 
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelPlayer1", "Player 1 : (BLACK)", 200);
	vecPlayerLabels[gamePlayers::P_WHITE] = 
		mTrayMgr->createLabel(OgreBites::TL_TOP, "labelPlayer2", "Player 2 : (WHITE)", 200);
	mTrayMgr->removeWidgetFromTray(vecPlayerLabels[gamePlayers::P_WHITE]);
	vecPlayerLabels[gamePlayers::P_WHITE]->hide();

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

    mRoot->addFrameListener(this);
}
//-----------------------------------------------------------------------------
void GomokuGame::createScene(void) {
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0, 0, 0));
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);

	Ogre::Entity* entRobot = mSceneMgr->createEntity("Ninja", "robot.mesh");
	entRobot->setCastShadows(true);
	Ogre::SceneNode* nodeRobot = mSceneMgr->getRootSceneNode()->createChildSceneNode(Ogre::Vector3(0, 0, -3));
	nodeRobot->attachObject(entRobot);
	nodeRobot->setScale(Ogre::Vector3(.05f, .05f, .05f));
	nodeRobot->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(-3.14f / 2.0f));

	//BULLET
	shapeStone = new btCylinderShape(btVector3(0.075f, 0.025f, 0.075f));

	Ogre::Entity* entTable = mSceneMgr->createEntity("GomokuTable", "GomokuTable.mesh");
	Ogre::SceneNode* nodeTable = mSceneMgr->getRootSceneNode()->createChildSceneNode("GomokuTable", Ogre::Vector3(0, 10, 0));
	nodeTable->attachObject(entTable);
	entTable->setCastShadows(true);
	nodeTable->setScale(Ogre::Vector3(2, 2, 2));
	//BULLET
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
	//stop until new game started
	rigidTable->setActivationState(0);

	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane, 200, 200, 10, 10, true, 1, 40, 40, Ogre::Vector3::UNIT_Z);
	Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
	Ogre::SceneNode* nodeGround = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nodeGround->attachObject(entGround);
	entGround->setMaterialName("Examples/Rockwall");
	entGround->setCastShadows(false);
	//BULLET
	btTransform transformGround;
	transformGround.setIdentity();
	btScalar massGround = 0;
	btVector3 localGroundInertia(0, 0, 0);
	shapeGround = new btBoxShape(btVector3(btScalar(60), btScalar(1), btScalar(60)));
	transformGround.setOrigin(btVector3(0, -1, 0));
	motionGround = new btDefaultMotionState(transformGround);
	shapeGround->calculateLocalInertia(massGround, localGroundInertia);
	btRigidBody::btRigidBodyConstructionInfo groundRBInfo(massGround, motionGround, shapeGround, localGroundInertia);
	rigidGround = new btRigidBody(groundRBInfo);
	rigidGround->setFriction(5);
	physicsEngine->getDynamicsWorld()->addRigidBody(rigidGround);

	//Selector Stone
	Ogre::SceneNode* pickNode = nodeTable->createChildSceneNode("PickNode");
	Ogre::Entity* entStone = mSceneMgr->createEntity("PickStone", "GomokuStonePick.mesh");
	pickNode->attachObject(entStone);
	pickNode->setScale(0.081f, 0.081f, 0.081f);

	//LIGHTS
	Ogre::Light* directionalLight = mSceneMgr->createLight("directionalLight");
	directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
	directionalLight->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setDirection(Ogre::Vector3(0, -1, -0.3f));

	//ambient light (darkness)
	mSceneMgr->setAmbientLight(Ogre::ColourValue(.5f, .5f, .5f));

	//fog & sky
	mWindow->getViewport(0)->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	mSceneMgr->setFog(Ogre::FOG_EXP2, Ogre::ColourValue(0, 0, 0), 0.07f);
}
//-------------------------------------------------------------------------------------
void GomokuGame::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void GomokuGame::createViewports(void)
{
	// Create one viewport, entire window
	Ogre::Viewport* vp = mWindow->addViewport(mCamera);
	vp->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
}
//-------------------------------------------------------------------------------------
void GomokuGame::setupResources(void)
{
    // Load resource paths from config file
    Ogre::ConfigFile cf;
    cf.load(mResourcesCfg);

    // Go through all sections & settings in the file
    Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

    Ogre::String secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}
//-------------------------------------------------------------------------------------
void GomokuGame::createResourceListener(void)
{

}
//-------------------------------------------------------------------------------------
void GomokuGame::loadResources(void)
{
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}
//-------------------------------------------------------------------------------------
void GomokuGame::go(void)
{
#ifdef _DEBUG
    mResourcesCfg = "resources_d.cfg";
    mPluginsCfg = "plugins_d.cfg";
#else
    mResourcesCfg = "resources.cfg";
    mPluginsCfg = "plugins.cfg";
#endif

    if (!setup())
        return;

    mRoot->startRendering();

    // clean up
    destroyScene();
}
//-------------------------------------------------------------------------------------
bool GomokuGame::setup(void)
{
	mRoot = new Ogre::Root("", "", "LowLevelOgre.log");

	Ogre::StringVector required_plugins;
	required_plugins.push_back("GL RenderSystem");
	required_plugins.push_back("Octree & Terrain Scene Manager");

	Ogre::StringVector plugins_toLoad;
	plugins_toLoad.push_back("RenderSystem_GL");
	plugins_toLoad.push_back("Plugin_ParticleFX");
	plugins_toLoad.push_back("Plugin_BSPSceneManager");
	//plugins_toLoad.push_back("Plugin_CgProgramManager");

	for (Ogre::StringVector::iterator j = plugins_toLoad.begin(); j != plugins_toLoad.end(); j++)
	{
#ifdef _DEBUG
		mRoot->loadPlugin(*j + Ogre::String("_d"));
#else
		mRoot->loadPlugin(*j);
#endif;
	}

    setupResources();

	physicsEngine = new Physics();
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

    bool carryOn = configure();
    if (!carryOn) return false;

    chooseSceneManager();
    createCamera();
    createViewports();

    // Set default mipmap level (NB some APIs ignore this)
    Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);

    // Create any resource listeners (for loading screens)
    createResourceListener();
    // Load resources
    loadResources();

    // Create the scene
    createScene();

    createFrameListener();

    return true;
};
//-------------------------------------------------------------------------------------
bool GomokuGame::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    if(mWindow->isClosed())
        return false;

    if(mShutDown)
        return false;
	
	//step physics
	if (physicsEngine != NULL) {
		physicsEngine->getDynamicsWorld()->stepSimulation(evt.timeSinceLastFrame);

		btCollisionObjectArray arrObj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray();

		for (int i = 0; i < arrObj.size(); i++) {
			btCollisionObject* obj = arrObj[i];
			btRigidBody* body = btRigidBody::upcast(obj);

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

	//start next turn when board is ready
	if (rigidTable->getLinearVelocity().length() > 0.1f) {
		bHasFallen = true;
	}
	if (!bGameStart && bHasFallen && rigidTable->getLinearVelocity().length() < 0.001f) {
		bGameStart = true;
		nextTurn();
	}

	float timeWait = 0.1f;
	if (bGameStart && bGameAIVAI && !bGameOver) {
		mAITurnTimer += evt.timeSinceLastFrame;
		if (mAITurnTimer >= timeWait) {
			mAITurnTimer = 0.0f;
			nextTurn();
		}
	}

	mInputContext.capture();

	bool bVisible = true;
	if (!mOnBoard || !gBoard.emptyTile(mBoardX, mBoardY)) bVisible = false;
	mSceneMgr->getSceneNode("PickNode")->setVisible(bVisible);

    mTrayMgr->frameRenderingQueued(evt);

    if (!mTrayMgr->isDialogVisible())
    {
        mCameraMan->frameRenderingQueued(evt);   // if dialog isn't up, then update the camera
        if (mDetailsPanel->isVisible())   // if details panel is visible, then update its contents
        {
            mDetailsPanel->setParamValue(0, Ogre::StringConverter::toString(mCamera->getDerivedPosition().x));
            mDetailsPanel->setParamValue(1, Ogre::StringConverter::toString(mCamera->getDerivedPosition().y));
            mDetailsPanel->setParamValue(2, Ogre::StringConverter::toString(mCamera->getDerivedPosition().z));
            mDetailsPanel->setParamValue(4, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().w));
            mDetailsPanel->setParamValue(5, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().x));
            mDetailsPanel->setParamValue(6, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().y));
            mDetailsPanel->setParamValue(7, Ogre::StringConverter::toString(mCamera->getDerivedOrientation().z));
        }
    }

    return true;
}
//-------------------------------------------------------------------------------------
bool GomokuGame::keyPressed( const OIS::KeyEvent &arg )
{
    if (mTrayMgr->isDialogVisible()) return true;   // don't process any more keys if dialog is up

    if (arg.key == OIS::KC_F)   // toggle visibility of advanced frame stats
    {
        mTrayMgr->toggleAdvancedFrameStats();
    }
    else if (arg.key == OIS::KC_G)   // toggle visibility of even rarer debugging details
    {
        if (mDetailsPanel->getTrayLocation() == OgreBites::TL_NONE)
        {
            mTrayMgr->moveWidgetToTray(mDetailsPanel, OgreBites::TL_TOPRIGHT, 0);
            mDetailsPanel->show();
        }
        else
        {
            mTrayMgr->removeWidgetFromTray(mDetailsPanel);
            mDetailsPanel->hide();
        }
    }
    else if (arg.key == OIS::KC_T)   // cycle polygon rendering mode
    {
        Ogre::String newVal;
        Ogre::TextureFilterOptions tfo;
        unsigned int aniso;

        switch (mDetailsPanel->getParamValue(9).asUTF8()[0])
        {
        case 'B':
            newVal = "Trilinear";
            tfo = Ogre::TFO_TRILINEAR;
            aniso = 1;
            break;
        case 'T':
            newVal = "Anisotropic";
            tfo = Ogre::TFO_ANISOTROPIC;
            aniso = 8;
            break;
        case 'A':
newVal = "None";
tfo = Ogre::TFO_NONE;
aniso = 1;
break;
        default:
			newVal = "Bilinear";
			tfo = Ogre::TFO_BILINEAR;
			aniso = 1;
		}

		Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(tfo);
		Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(aniso);
		mDetailsPanel->setParamValue(9, newVal);
	}
	else if (arg.key == OIS::KC_R)   // cycle polygon rendering mode
	{
		Ogre::String newVal;
		Ogre::PolygonMode pm;

		switch (mCamera->getPolygonMode())
		{
		case Ogre::PM_SOLID:
			newVal = "Wireframe";
			pm = Ogre::PM_WIREFRAME;
			break;
		case Ogre::PM_WIREFRAME:
			newVal = "Points";
			pm = Ogre::PM_POINTS;
			break;
		default:
			newVal = "Solid";
			pm = Ogre::PM_SOLID;
		}

		mCamera->setPolygonMode(pm);
		mDetailsPanel->setParamValue(10, newVal);
	}
	else if (arg.key == OIS::KC_F5)   // refresh all textures
	{
		Ogre::TextureManager::getSingleton().reloadAll();
	}
	else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
	{
		mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
	}
	else if (arg.key == OIS::KC_ESCAPE)
	{
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
	else if (arg.key == OIS::KC_O) {
		/*
		int size = gBoard.getBoardSize();
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				std::string stre = "fill" + std::to_string(i) + "_" + std::to_string(j);
				std::string strn = "fillNode" + std::to_string(i) + "_" + std::to_string(j);
				int color = rand() % 2 + 1;

				if (gBoard.addStone(i, j, color, stre, strn)) {
					Ogre::Entity* entStone;
					if (color == stoneColor::BLACK) {
						entStone = mSceneMgr->createEntity(stre, "GomokuStoneBlack.mesh");
					}
					else {
						entStone = mSceneMgr->createEntity(stre, "GomokuStoneWhite.mesh");
					}
					Ogre::SceneNode* tableNode = mSceneMgr->getSceneNode("GomokuTable");
					Ogre::SceneNode* stoneNode = tableNode->createChildSceneNode(strn,
						(Ogre::Vector3((float)i * 0.086f - 0.61f, 0.1f, (float)j * 0.088f - 0.61f)));
					stoneNode->attachObject(entStone);
					stoneNode->setScale(0.081f, 0.081f, 0.081f);
				}
			}
		}*/
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
	mPickCoords = getGameLookCoords();
	int size = gBoard.getBoardSize();
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

	mPickCoords = Ogre::Vector3(mBoardX * 0.086f - 0.61f, pY, mBoardY * 0.086f - 0.61f);

	Ogre::SceneNode* pickNode = mSceneMgr->getSceneNode("PickNode");
	pickNode->setPosition(mPickCoords);

	if (mCursorMode && mTrayMgr->injectMouseMove(arg)) return true;
	else if (!mCursorMode) mCameraMan->injectMouseMove(arg);
	return true;
}

bool GomokuGame::mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
	if (id == OIS::MB_Left) {
		if (mOnBoard && !mCursorMode && !bGameOver && !bGameAIVAI && bGameStart) {
			//add stone if able to
			addStoneToBoard(mBoardX, mBoardY);
		}
	}
	else if (id == OIS::MB_Right) {
		//shootBox();
	}

    if (mCursorMode && mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GomokuGame::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mCursorMode && mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
}

void GomokuGame::buttonHit(OgreBites::Button * button)
{
	if (button->getName() == "buttonQuit") {
		mShutDown = true;
	}
	else if (button->getName() == "buttonResume") {
		setMenu(menuState::CLOSED);
	}
	else if (button->getName() == "buttonNew") {
		bGameOver = true;
		rigidTable->activate();
		setStonePhysics();
		setMenu(menuState::NEW_GAME);
	}
	else if (button->getName() == "buttonNewAI") {
		bGameVSAI = true;
		bGameAIVAI = false;
		resetGame();
		setMenu(menuState::CLOSED);
	}
	else if (button->getName() == "buttonNewHum") {
		bGameVSAI = false;
		bGameAIVAI = false;
		resetGame();
		setMenu(menuState::CLOSED);
	}
	else if (button->getName() == "buttonNewAIvAI") {
		bGameVSAI = false;
		bGameAIVAI = true;
		resetGame();
		setMenu(menuState::CLOSED);
	}
}

void GomokuGame::setMenu(int state) {
	removeAllMenuItems(mTrayMgr->getTrayContainer(OgreBites::TL_CENTER));

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

//Adjust mouse clipping area
void GomokuGame::windowResized(Ogre::RenderWindow* rw)
{
    unsigned int width, height, depth;
    int left, top;
    rw->getMetrics(width, height, depth, left, top);

    const OIS::MouseState &ms = mMouse->getMouseState();
    ms.width = width;
    ms.height = height;
}

//Unattach OIS before window shutdown (very important under Linux)
void GomokuGame::windowClosed(Ogre::RenderWindow* rw)
{
    //Only close for window that created OIS (the main window in these demos)
    if( rw == mWindow )
    {
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
	Ogre::Vector3 dir = mCamera->getDirection();
	Ogre::Vector3 pos = mCamera->getPosition();
	float baseAngle = Ogre::Vector3::UNIT_X.angleBetween(Ogre::Vector3(dir.x, 0, dir.z)).valueRadians();
	float baseDist = Ogre::Vector3::NEGATIVE_UNIT_Y.distance(dir);
	baseDist = baseDist * pos.y;
	
	Ogre::Real offsetX = baseDist * cos(baseAngle);
	Ogre::Real offsetZ = baseDist * -sin(baseAngle);

	Ogre::Vector3 coords = Ogre::Vector3(offsetX + pos.x, 0.1f, offsetZ + pos.z);

	mDetailsPanel->setParamValue(12, Ogre::StringConverter::toString(coords.x));
	mDetailsPanel->setParamValue(13, Ogre::StringConverter::toString(coords.z));

	return coords;
}

void GomokuGame::setStonePhysics() {
	//flip board if physics not already applied
	if (!bPhysicsApplied) {
		std::vector<GameTile> vecStones = gBoard.getAllStones();

		for (size_t i = 0; i < vecStones.size(); i++) {
			Ogre::SceneNode* nodeStone = mSceneMgr->getSceneNode(vecStones[i].nodeName);
			Ogre::Vector3 pos = nodeStone->_getDerivedPosition();
			mSceneMgr->destroyEntity(vecStones[i].entName);
			mSceneMgr->destroySceneNode(vecStones[i].nodeName);

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

		rigidTable->applyImpulse(btVector3(0, 30, 50), btVector3(0, 0, 10));
	}

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
			displayWinner(gameWinners::WIN_TIE);
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
	if ((bGameVSAI || bGameAIVAI) && mCurrentPlayer == playerAI.getPlayerNum() && bGameStart) {
		TilePos* move;
		do {
			move = playerAI.getNextMove();
		} while (!addStoneToBoard(move->xGrid, move->yGrid));
	}
	else if (bGameAIVAI && mCurrentPlayer == playerAI2.getPlayerNum() && bGameStart) {
		TilePos* move;
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
		playerAI.setPlayerNum(playerAINum, newColor);

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
			playerAI2.setPlayerNum(playerAINum2, newColor);
		}
	}
	setPlayerLabel(true);

	//clear winner from screen
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
	for (size_t i = 0; i < vecMenuButtons.size(); i++) {
		mTrayMgr->removeWidgetFromTray(vecMenuButtons[i]->getName());
		vecMenuButtons[i]->hide();
	}
}

void GomokuGame::displayWinner(int player)
{
	mTrayMgr->moveWidgetToTray(vecWinnerLabels[player], OgreBites::TL_TOP);
	vecWinnerLabels[player]->show();
	
	mTrayMgr->removeWidgetFromTray(vecPlayerLabels[mCurrentPlayer]);
	vecPlayerLabels[mCurrentPlayer]->hide();
}

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
{
	GomokuGame game;

	try {
		game.go();
	}
	catch (Ogre::Exception& e) {
		MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}

	return 0;
}