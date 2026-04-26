#include "ExtraLife.h"
#include "BoundingShape.h"

ExtraLife::ExtraLife(void) : GameObject("ExtraLife")
{
	// Random position on screen
	mPosition.x = (rand() % 160) - 80;
	mPosition.y = (rand() % 120) - 60;
	mPosition.z = 0.0;
	mVelocity.x = 0.0;
	mVelocity.y = 0.0;
	mVelocity.z = 0.0;
}

ExtraLife::~ExtraLife(void)
{
}

bool ExtraLife::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Spaceship")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void ExtraLife::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}