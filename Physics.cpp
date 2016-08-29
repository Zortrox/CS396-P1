#include "Physics.h"

Physics::Physics() {
	initObjects();
}
Physics::~Physics() {

}

void Physics::initObjects() {
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));
}

btDiscreteDynamicsWorld* Physics::getDynamicsWorld() {
	return dynamicsWorld;
}

std::vector<btCollisionShape*> Physics::getCollisionShapes() {
	return collisionShapes;
}