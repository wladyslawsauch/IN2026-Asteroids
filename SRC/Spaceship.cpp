#include "GameUtil.h"
#include "GameWorld.h"
#include "Bullet.h"
#include "Spaceship.h"
#include "BoundingSphere.h"

using namespace std;

Spaceship::Spaceship()
	: GameObject("Spaceship"), mThrust(0),
	mInvulnerable(false), mInvulnerabilityTimer(0),
	mWeaponUpgrade(false), mWeaponUpgradeTimer(0),
	mCanShoot(true), mShootCooldown(0)
{
}

Spaceship::Spaceship(GLVector3f p, GLVector3f v, GLVector3f a, GLfloat h, GLfloat r)
	: GameObject("Spaceship", p, v, a, h, r), mThrust(0),
	mInvulnerable(false), mInvulnerabilityTimer(0),
	mWeaponUpgrade(false), mWeaponUpgradeTimer(0),
	mCanShoot(true), mShootCooldown(0)
{
}

Spaceship::Spaceship(const Spaceship& s)
	: GameObject(s), mThrust(0),
	mInvulnerable(false), mInvulnerabilityTimer(0),
	mWeaponUpgrade(false), mWeaponUpgradeTimer(0),
	mCanShoot(true), mShootCooldown(0)
{
}

Spaceship::~Spaceship(void)
{
}

void Spaceship::Reset(void)
{
	// Call parent reset
	GameObject::Reset();
	// Reset thrust and powerups
	mThrust = 0;
	mInvulnerable = false;
	mInvulnerabilityTimer = 0;
	mWeaponUpgrade = false;
	mWeaponUpgradeTimer = 0;

	mCanShoot = true;
	mShootCooldown = 0;
}

void Spaceship::Update(int t)
{
	GameObject::Update(t);

	// Count down invulnerability timer
	if (mInvulnerable)
	{
		mInvulnerabilityTimer -= t;
		if (mInvulnerabilityTimer <= 0)
		{
			mInvulnerable = false;
			mInvulnerabilityTimer = 0;
		}
	}

	// Count down weapon upgrade timer
	if (mWeaponUpgrade)
	{
		mWeaponUpgradeTimer -= t;
		if (mWeaponUpgradeTimer <= 0)
		{
			mWeaponUpgrade = false;
			mWeaponUpgradeTimer = 0;
		}
	}

	if (!mCanShoot)
	{
		mShootCooldown -= t;
		if (mShootCooldown <= 0)
		{
			mCanShoot = true;
			mShootCooldown = 0;
		}
	}
}

void Spaceship::Render(void)
{
	if (mSpaceshipShape.get() != NULL) mSpaceshipShape->Render();

	if ((mThrust > 0) && (mThrusterShape.get() != NULL))
		mThrusterShape->Render();

	GameObject::Render();
}

void Spaceship::Thrust(float t)
{
	mThrust = t;
	mAcceleration.x = mThrust * cos(DEG2RAD * mAngle);
	mAcceleration.y = mThrust * sin(DEG2RAD * mAngle);
}

void Spaceship::Rotate(float r)
{
	mRotation = r;
}

void Spaceship::Shoot(void)
{

	if (!mCanShoot) return;
	mCanShoot = false;
	mShootCooldown = 300;
	if (!mWorld) return;

	GLVector3f heading(cos(DEG2RAD * mAngle), sin(DEG2RAD * mAngle), 0);
	heading.normalize();
	GLVector3f bullet_position = mPosition + (heading * 4);
	float bullet_speed = 30;
	GLVector3f bullet_velocity = mVelocity + heading * bullet_speed;

	// Normal bullet
	shared_ptr<GameObject> bullet(
		new Bullet(bullet_position, bullet_velocity, mAcceleration, mAngle, 0, 2000));
	bullet->SetBoundingShape(make_shared<BoundingSphere>(bullet->GetThisPtr(), 2.0f));
	bullet->SetShape(mBulletShape);
	mWorld->AddObject(bullet);

	// Weapon upgrade - shoot 2 extra bullets at angles
	if (mWeaponUpgrade)
	{
		float angles[2] = { mAngle + 15.0f, mAngle - 15.0f };
		for (int i = 0; i < 2; i++)
		{
			GLVector3f h2(cos(DEG2RAD * angles[i]), sin(DEG2RAD * angles[i]), 0);
			h2.normalize();
			GLVector3f bp2 = mPosition + (h2 * 4);
			GLVector3f bv2 = mVelocity + h2 * bullet_speed;
			shared_ptr<GameObject> b2(
				new Bullet(bp2, bv2, mAcceleration, angles[i], 0, 2000));
			b2->SetBoundingShape(make_shared<BoundingSphere>(b2->GetThisPtr(), 2.0f));
			b2->SetShape(mBulletShape);
			mWorld->AddObject(b2);
		}
	}
}

void Spaceship::ActivateInvulnerability(int duration_ms)
{
	mInvulnerable = true;
	mInvulnerabilityTimer = duration_ms;
}

void Spaceship::ActivateWeaponUpgrade(int duration_ms)
{
	mWeaponUpgrade = true;
	mWeaponUpgradeTimer = duration_ms;
}

bool Spaceship::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Asteroid")) return false;
	if (mInvulnerable) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void Spaceship::OnCollision(const GameObjectList& objects)
{
	// Only die if collided with an asteroid
	for (GameObjectList::const_iterator it = objects.begin();
		it != objects.end(); ++it)
	{
		if ((*it)->GetType() == GameObjectType("Asteroid"))
		{
			mWorld->FlagForRemoval(GetThisPtr());
			return;
		}
	}
}