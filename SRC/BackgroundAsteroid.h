#ifndef __BACKGROUNDASTEROID_H__
#define __BACKGROUNDASTEROID_H__

#include "GameObject.h"

class BackgroundAsteroid : public GameObject
{
public:
	BackgroundAsteroid(void);
	~BackgroundAsteroid(void);

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
};

#endif