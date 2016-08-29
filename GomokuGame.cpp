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
    mCursorWasVisible(false),
    mShutDown(false),
    mInputManager(0),
    mMouse(0),
    mKeyboard(0),
	mOverlaySystem(0)
{
	physicsEngine = new Physics();
	numBoxes = 0;
	srand(time(NULL));
	mBoardX = 0;
	mBoardY = 0;
	mOnBoard = false;
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
        mWindow = mRoot->initialise(true, "TutorialApplication Render Window");

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

	//Fix for 1.9 - take this out:
	/*OgreBites::InputContext inputContext;
	inputContext.mMouse = mMouse; 
	inputContext.mKeyboard = mKeyboard;
	mTrayMgr = new OgreBites::SdkTrayManager("TrayMgr", mWindow, inputContext, this);*/

	/*OgreBites::InputContext input;
	input.mAccelerometer = NULL;
	input.mKeyboard = mKeyboard;
	input.mMouse = mMouse;
	input.mMultiTouch = NULL;
	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, input, this);*/

	//Fix for 1.9 - put this in:
	mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mInputContext, this);
	//mTrayMgr = new OgreBites::SdkTrayManager("InterfaceName", mWindow, mMouse, this);
    mTrayMgr->showFrameStats(OgreBites::TL_BOTTOMLEFT);
    mTrayMgr->showLogo(OgreBites::TL_BOTTOMRIGHT);
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

	Ogre::Entity* entTable = mSceneMgr->createEntity("GomokuTable", "GomokuTable.mesh");
	Ogre::SceneNode* nodeTable = mSceneMgr->getRootSceneNode()->createChildSceneNode("GomokuTable", Ogre::Vector3(0, 10, 0));
	nodeTable->attachObject(entTable);
	entTable->setCastShadows(true);
	nodeTable->setScale(Ogre::Vector3(2, 2, 2));
	//BULLET
	btCollisionShape *boxShape = new btBoxShape(btVector3(2.0f, 0.15f, 2.0f)); //btConvexTriangleMeshShape(tMeshTable);
	physicsEngine->getCollisionShapes().push_back(boxShape);
	btTransform startTransform;
	startTransform.setIdentity();
	//startTransform.setRotation(btQuaternion(1.0f, 1.0f, 1.0f, 0));
	btScalar mass = 10.0f;
	btVector3 localInertia(0, 0, 0);
	startTransform.setOrigin(btVector3(0, 10, 0));
	boxShape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, boxShape, localInertia);
	btRigidBody *boxBody = new btRigidBody(rbInfo);
	boxBody->setRestitution(1);
	boxBody->setFriction(2);
	boxBody->setUserPointer(nodeTable);
	physicsEngine->getDynamicsWorld()->addRigidBody(boxBody);

	Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
	Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
		plane, 15, 15, 20, 20, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
	Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
	Ogre::SceneNode* nodeGround = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	nodeGround->attachObject(entGround);
	entGround->setMaterialName("Examples/Rockwall");
	entGround->setCastShadows(false);
	//BULLET
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -1, 0));
	btScalar groundMass(0);
	btVector3 localGroundInertia(0, 0, 0);
	btCollisionShape *groundShape = new btBoxShape(btVector3(btScalar(15), btScalar(1), btScalar(15)));
	btDefaultMotionState *groundMotionState = new btDefaultMotionState(groundTransform);
	groundShape->calculateLocalInertia(groundMass, localGroundInertia);
	btRigidBody::btRigidBodyConstructionInfo groundRBInfo(groundMass, groundMotionState, groundShape, localGroundInertia);
	btRigidBody *groundBody = new btRigidBody(groundRBInfo);
	groundBody->setFriction(5);
	physicsEngine->getDynamicsWorld()->addRigidBody(groundBody);

	//LIGHTS
	Ogre::Light* directionalLight = mSceneMgr->createLight("directionalLight");
	directionalLight->setType(Ogre::Light::LT_DIRECTIONAL);
	directionalLight->setDiffuseColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setSpecularColour(Ogre::ColourValue(1.0f, 1.0f, 1.0f));
	directionalLight->setDirection(Ogre::Vector3(0.7f, -1, 0.5f));

	//ambient light (brighten it up a little)
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	createGameArea(nodeTable);
}
//-------------------------------------------------------------------------------------
void GomokuGame::destroyScene(void)
{
}
//-------------------------------------------------------------------------------------
void GomokuGame::createGameArea(Ogre::SceneNode* tableNode) {
	vecGameArea.resize(15);
	for (size_t i = 0; i < vecGameArea.size(); i++) {
		vecGameArea[i].resize(15);
		for (size_t j = 0; j < vecGameArea[i].size(); j++) {
			vecGameArea[i][j] = stoneColor::NONE;
			std::string strNum = std::to_string(i) + "_" + std::to_string(j);
			Ogre::SceneNode* child = tableNode->createChildSceneNode("nodeStone_" + strNum, (Ogre::Vector3((float)i * 0.086f - 0.61f, 0.1f, (float)j * 0.088f - 0.61f)));
		}
	}

	Ogre::SceneNode* pickNode = tableNode->createChildSceneNode("PickNode");
	Ogre::Entity* entStone = mSceneMgr->createEntity("PickStone", "GomokuStonePick.mesh");
	pickNode->attachObject(entStone);
	pickNode->setScale(0.081f, 0.081f, 0.081f);
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
	plugins_toLoad.push_back("Plugin_CgProgramManager");

	for (Ogre::StringVector::iterator j = plugins_toLoad.begin(); j != plugins_toLoad.end(); j++)
	{
#ifdef _DEBUG
		mRoot->loadPlugin(*j + Ogre::String("_d"));
#else
		mRoot->loadPlugin(*j);
#endif;
	}

    setupResources();

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

		for (int i = 0; i < 2+numBoxes; i++) {
			btCollisionObject* obj = physicsEngine->getDynamicsWorld()->getCollisionObjectArray()[i];
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

	mInputContext.capture();

	bool bVisible = true;
	if (!mOnBoard || vecGameArea[mBoardX][mBoardY] != stoneColor::NONE) bVisible = false;
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
    else if(arg.key == OIS::KC_F5)   // refresh all textures
    {
        Ogre::TextureManager::getSingleton().reloadAll();
    }
    else if (arg.key == OIS::KC_SYSRQ)   // take a screenshot
    {
        mWindow->writeContentsToTimestampedFile("screenshot", ".jpg");
    }
    else if (arg.key == OIS::KC_ESCAPE)
    {
        mShutDown = true;
    }

    mCameraMan->injectKeyDown(arg);
    return true;
}

bool GomokuGame::keyReleased( const OIS::KeyEvent &arg )
{
    mCameraMan->injectKeyUp(arg);
    return true;
}

bool GomokuGame::mouseMoved( const OIS::MouseEvent &arg )
{
	mPickCoords = getGameLookCoords();
	float pX = floor((mPickCoords.x + .6501f)/1.203f * 14);
	float pY = mPickCoords.y;
	float pZ = floor((mPickCoords.z + .651f) / 1.232f * 14);
	mBoardX = pX;
	mBoardY = pZ;

	//hide the cursor if not looking at the board
	if (mBoardX < 0 || mBoardX > 14 || mBoardY < 0 || mBoardY > 14) {
		mOnBoard = false;
	}
	else {
		mOnBoard = true;
	}

	mPickCoords = Ogre::Vector3(mBoardX * 0.086f - 0.61f, pY, mBoardY * 0.088f - 0.61f);

	Ogre::SceneNode* pickNode = mSceneMgr->getSceneNode("PickNode");
	pickNode->setPosition(mPickCoords);
	
	if (mTrayMgr->injectMouseMove(arg)) return true;
    mCameraMan->injectMouseMove(arg);
    return true;
}

bool GomokuGame::mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
	if (id == OIS::MB_Left) {
		if (mOnBoard && vecGameArea[mBoardX][mBoardY] == stoneColor::NONE) {
			//place stone at location
			vecGameArea[mBoardX][mBoardY] = stoneColor::BLACK;
			std::string strNum = std::to_string(mBoardX) + "_" + std::to_string(mBoardY);
			Ogre::Entity* entStone = mSceneMgr->createEntity("Stone" + strNum, "GomokuStoneBlack.mesh");
			Ogre::SceneNode* stoneNode = mSceneMgr->getSceneNode("nodeStone_" + strNum);
			stoneNode->attachObject(entStone);
			stoneNode->setScale(0.081f, 0.081f, 0.081f);
		}

		//shootBox();
	}
    if (mTrayMgr->injectMouseDown(arg, id)) return true;
    mCameraMan->injectMouseDown(arg, id);
    return true;
}

bool GomokuGame::mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id )
{
    if (mTrayMgr->injectMouseUp(arg, id)) return true;
    mCameraMan->injectMouseUp(arg, id);
    return true;
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

void GomokuGame::shootBox() {
	Ogre::Vector3 pos = mCamera->getPosition();
	Ogre::Vector3 dir = mCamera->getDirection();

	float xDir = dir.x * 5.0f;
	float yDir = dir.y * 5.0f;
	float zDir = dir.z * 5.0f;

	std::string strNum = std::to_string(numBoxes);

	Ogre::Entity* entStone;

	if (rand() % 2 == 0) {
		entStone = mSceneMgr->createEntity("Stone_" + strNum, "GomokuStoneBlack.mesh");
	}
	else {
		entStone = mSceneMgr->createEntity("Stone_" + strNum, "GomokuStoneWhite.mesh");
	}

	Ogre::SceneNode* nodeStone = mSceneMgr->getRootSceneNode()->createChildSceneNode(pos);
	nodeStone->attachObject(entStone);
	entStone->setCastShadows(true);
	nodeStone->setScale(0.15f, 0.15f, 0.15f);

	//BULLET
	btCollisionShape *boxShape = new btCylinderShape(btVector3(0.075f, 0.025f, 0.075f));
	physicsEngine->getCollisionShapes().push_back(boxShape);
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setRotation(btQuaternion(1.0f, 1.0f, 1.0f, 0));
	btScalar mass = .02f;
	btVector3 localInertia(0, 0, 0);
	startTransform.setOrigin(btVector3(pos.x, pos.y, pos.z));
	boxShape->calculateLocalInertia(mass, localInertia);
	btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, boxShape, localInertia);
	btRigidBody *stoneBody = new btRigidBody(rbInfo);
	stoneBody->setLinearVelocity(btVector3(xDir, yDir, zDir));
	stoneBody->setAngularVelocity(btVector3((float)(rand() % 5 - 2), (float)(rand() % 5 - 2), (float)(rand() % 5 - 2)));
	stoneBody->setRestitution(0.1f);
	stoneBody->setUserPointer(nodeStone);
	stoneBody->setFriction(1.5);
	physicsEngine->getDynamicsWorld()->addRigidBody(stoneBody);

	numBoxes++;
}

Ogre::Vector3 GomokuGame::getGameLookCoords() {
	Ogre::Vector3 dir = mCamera->getDirection();
	Ogre::Vector3 pos = mCamera->getPosition();
	float angX = Ogre::Vector3::NEGATIVE_UNIT_Y.angleBetween(Ogre::Vector3(dir.x, dir.y, 0)).valueRadians();
	float angZ = Ogre::Vector3::NEGATIVE_UNIT_Y.angleBetween(Ogre::Vector3(0, dir.y, dir.z)).valueRadians();

	Ogre::Real offsetX = (pos.y - 0.1f) * tan(angX) * (dir.x < 0 ? -1 : 1);
	Ogre::Real offsetZ = (pos.y - 0.1f) * tan(angZ) * (dir.z < 0 ? -1 : 1);

	Ogre::Vector3 coords = Ogre::Vector3(offsetX + pos.x, 0.1f, offsetZ + pos.z);

	mDetailsPanel->setParamValue(12, Ogre::StringConverter::toString(angX));
	mDetailsPanel->setParamValue(13, Ogre::StringConverter::toString(angZ));

	return coords;
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