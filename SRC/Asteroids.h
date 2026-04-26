#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"

class GameObject;
class Spaceship;
class GUILabel;

struct HighScoreEntry {
	std::string name;
	int score;
	HighScoreEntry() : score(0) {}
};

enum GameState {
	STATE_MENU,
	STATE_INSTRUCTIONS,
	STATE_HIGHSCORES,
	STATE_PLAYING,
	STATE_ENTER_NAME,
	STATE_GAME_OVER
};

enum MenuItem {
	MENU_START = 0,
	MENU_DIFFICULTY = 1,
	MENU_INSTRUCTIONS = 2,
	MENU_HIGHSCORES = 3,
	MENU_COUNT = 4
};

class Asteroids : public GameSession, public IKeyboardListener,
	public IGameWorldListener, public IScoreListener,
	public IPlayerListener
{
public:
	Asteroids(int argc, char* argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	void OnScoreChanged(int score);
	void OnPlayerKilled(int lives_left);

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	void OnTimer(int value);

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mGameOverLabel;
	shared_ptr<GUILabel> mTitleLabel;
	shared_ptr<GUILabel> mSubtitleLabel;
	shared_ptr<GUILabel> mMenuItemLabels[4];
	shared_ptr<GUILabel> mInfoLabels[8];
	shared_ptr<GUILabel> mPowerUpLabel;

	uint mLevel;
	uint mAsteroidCount;

	GameState mGameState;
	int mSelectedMenuItem;
	bool mDifficultyEnabled;
	bool mMenuAsteroidsCreated;

	static const int MAX_HIGH_SCORES = 5;
	HighScoreEntry mHighScores[MAX_HIGH_SCORES];
	int mHighScoreCount;
	int mCurrentScore;
	std::string mEnteredName;

	int mExtraLives;
	std::vector<weak_ptr<GameObject>> mBackgroundAsteroids;
	void RemoveBackgroundAsteroids();

	std::vector<weak_ptr<GameObject>> mGameAsteroids;
	void RemoveGameAsteroids();
	std::vector<weak_ptr<GameObject>> mPowerUpObjects;
	void RemovePowerUps();

	bool IsSpaceshipAlive();
	void StartGame();
	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	void CreateBackgroundAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();
	void SpawnPowerUps();
	void CreateExtraLife();
	void CreateInvulnerability();
	void CreateWeaponUpgrade();

	void ShowMenu();
	void ShowInstructions();
	void ShowHighScores();
	void ShowEnterName();
	void AddHighScore(const std::string& name, int score);
	void SaveHighScores();
	void LoadHighScores();
	void UpdateMenuLabel();
	void UpdateHighScoreLabel();
	void HideAllInfoLabels();
	void HideMenuLabels();

	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint SPAWN_POWERUPS = 3;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;
};

#endif