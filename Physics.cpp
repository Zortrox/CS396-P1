/*
	Physics.cpp
	Adds physics to the Ogre nodes

	@author Matthew Clark
	@date 09/02/2016
*/

#include "Physics.h"

Physics::Physics() {
	//create physics world
	initObjects();
}
Physics::~Physics() {
	//removes objects to clear RAM
	delete dynamicsWorld;
    delete solver;
    delete collisionConfiguration;
    delete dispatcher;
    delete overlappingPairCache;
}

void Physics::initObjects() {
	// These are just needed to set up a basic world
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	//sets the gravity (should be default -9.8, but I exlicitly set it)
	dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));
}

btDiscreteDynamicsWorld* Physics::getDynamicsWorld() {
	return dynamicsWorld;
}