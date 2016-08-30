#pragma once

#include "btBulletDynamicsCommon.h"
#include <vector>
#include <map>

class Physics {
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;
	std::vector<btCollisionShape*> collisionShapes;
	std::map<std::string, btRigidBody*> physicsAccessors;

public:
	Physics();
	~Physics();

	void initObjects();
	btDiscreteDynamicsWorld* getDynamicsWorld();
};