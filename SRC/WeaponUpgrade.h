#ifndef __WEAPONUPGRADE_H__
#define __WEAPONUPGRADE_H__

#include "GameObject.h"

class WeaponUpgrade : public GameObject
{
public:
	WeaponUpgrade(void);
	~WeaponUpgrade(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
};

#endif