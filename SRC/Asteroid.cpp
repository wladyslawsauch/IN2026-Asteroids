#include <stdlib.h>
#include "GameUtil.h"
#include "Asteroid.h"
#include "BoundingShape.h"

Asteroid::Asteroid(void) : GameObject("Asteroid")
{
	mAngle = rand() % 360;
	mRotation = 0;
	// Spawn at edge of world, not center
	int side = rand() % 4;
	if (side == 0) { mPosition.x = -100; mPosition.y = (rand() % 200) - 100; }
	else if (side == 1) { mPosition.x = 100; mPosition.y = (rand() % 200) - 100; }
	else if (side == 2) { mPosition.x = (rand() % 200) - 100; mPosition.y = -100; }
	else { mPosition.x = (rand() % 200) - 100; mPosition.y = 100; }
	mPosition.z = 0.0;
	mVelocity.x = 10.0 * cos(DEG2RAD * mAngle);
	mVelocity.y = 10.0 * sin(DEG2RAD * mAngle);
	mVelocity.z = 0.0;
}
Asteroid::~Asteroid(void)
{
}

bool Asteroid::CollisionTest(shared_ptr<GameObject> o)
{
	// Only collide with Spaceship and Bullet
	if (o->GetType() != GameObjectType("Spaceship") &&
		o->GetType() != GameObjectType("Bullet")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Asteroid::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}