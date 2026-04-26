#include <stdlib.h>
#include "GameUtil.h"
#include "BackgroundAsteroid.h"
#include "BoundingShape.h"

BackgroundAsteroid::BackgroundAsteroid(void) : GameObject("BackgroundAsteroid")
{
	mAngle = rand() % 360;
	mRotation = 0;
	mPosition.x = (rand() % 160) - 80;
	mPosition.y = (rand() % 120) - 60;
	mPosition.z = 0.0;
	mVelocity.x = 10.0 * cos(DEG2RAD * mAngle);
	mVelocity.y = 10.0 * sin(DEG2RAD * mAngle);
	mVelocity.z = 0.0;
}

BackgroundAsteroid::~BackgroundAsteroid(void)
{
}

bool BackgroundAsteroid::CollisionTest(shared_ptr<GameObject> o)
{
	return false;
}

void BackgroundAsteroid::OnCollision(const GameObjectList& objects)
{
}