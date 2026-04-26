#ifndef __SPACESHIP_H__
#define __SPACESHIP_H__

#include "GameUtil.h"
#include "GameObject.h"
#include "Shape.h"

class Spaceship : public GameObject
{
public:
	Spaceship();
	Spaceship(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r);
	Spaceship(const Spaceship& s);
	virtual ~Spaceship(void);

	virtual void Update(int t);
	virtual void Render(void);
	virtual void Reset(void);

	virtual void Thrust(float t);
	virtual void Rotate(float r);
	virtual void Shoot(void);

	void SetSpaceshipShape(shared_ptr<Shape> spaceship_shape) { mSpaceshipShape = spaceship_shape; }
	void SetThrusterShape(shared_ptr<Shape> thruster_shape) { mThrusterShape = thruster_shape; }
	void SetBulletShape(shared_ptr<Shape> bullet_shape) { mBulletShape = bullet_shape; }

	void ActivateInvulnerability(int duration_ms);
	void ActivateWeaponUpgrade(int duration_ms);
	bool IsInvulnerable() { return mInvulnerable; }
	bool HasWeaponUpgrade() { return mWeaponUpgrade; }

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);

private:
	float mThrust;

	bool mCanShoot;
	int mShootCooldown;

	shared_ptr<Shape> mSpaceshipShape;
	shared_ptr<Shape> mThrusterShape;
	shared_ptr<Shape> mBulletShape;

	bool mInvulnerable;
	int mInvulnerabilityTimer;
	bool mWeaponUpgrade;
	int mWeaponUpgradeTimer;
};

#endif