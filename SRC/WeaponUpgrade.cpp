#include "WeaponUpgrade.h"
#include "BoundingShape.h"

WeaponUpgrade::WeaponUpgrade(void) : GameObject("WeaponUpgrade")
{
	mPosition.x = (rand() % 160) - 80;
	mPosition.y = (rand() % 120) - 60;
	mPosition.z = 0.0;
	mVelocity.x = 0.0;
	mVelocity.y = 0.0;
	mVelocity.z = 0.0;
}

WeaponUpgrade::~WeaponUpgrade(void)
{
}

bool WeaponUpgrade::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Spaceship")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void WeaponUpgrade::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}