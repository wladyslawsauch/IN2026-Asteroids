#ifndef __SCOREKEEPER_H__
#define __SCOREKEEPER_H__

#include "GameUtil.h"
#include "GameObject.h"
#include "GameObjectType.h"
#include "IScoreListener.h"
#include "IGameWorldListener.h"

class ScoreKeeper : public IGameWorldListener
{
public:
	ScoreKeeper() { mScore = 0; mScoreEnabled = false; }
	virtual ~ScoreKeeper() {}

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}

	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
	{
		if (!mScoreEnabled) return;
		if (object->GetType() == GameObjectType("Asteroid")) {
			mScore += 10;
			FireScoreChanged();
		}
	}

	void AddListener(shared_ptr<IScoreListener> listener)
	{
		mListeners.push_back(listener);
	}

	void FireScoreChanged()
	{
		ScoreListenerList copy = mListeners;
		for (ScoreListenerList::iterator lit = copy.begin();
			lit != copy.end(); ++lit)
		{
			if (*lit) (*lit)->OnScoreChanged(mScore);
		}
	}

	void ResetScore()
	{
		mScore = 0;
	}

	void SetScoreEnabled(bool enabled) { mScoreEnabled = enabled; }

private:
	int mScore;
	bool mScoreEnabled;

	typedef std::list< shared_ptr<IScoreListener> > ScoreListenerList;
	ScoreListenerList mListeners;
};

#endif