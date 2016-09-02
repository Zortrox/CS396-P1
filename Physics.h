/*
	Physics.h
	Adds physics to the Ogre nodes

	@author Matthew Clark
	@date 09/02/2016
*/

#pragma once

#include "btBulletDynamicsCommon.h"
#include <vector>
#include <map>

class Physics {
public:
	Physics();
	~Physics();

	// Initializes the physics world for collisions
	void initObjects();

	// Returns the physics world for physics stepping and collision
	btDiscreteDynamicsWorld* getDynamicsWorld();

private:
	btDefaultCollisionConfiguration* collisionConfiguration; //configuration of world
	btCollisionDispatcher* dispatcher;						 //handles collision pair variables
	btBroadphaseInterface* overlappingPairCache;			 //handles overlapping collision points
	btSequentialImpulseConstraintSolver* solver;			 //impulse and force variables
	btDiscreteDynamicsWorld* dynamicsWorld;					 //actual physics world everything is tied to
};