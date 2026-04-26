#include <vector>
#include "Asteroid.h"
#include "Asteroids.h"
#include "BackgroundAsteroid.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "ExtraLife.h"
#include "Invulnerability.h"
#include "WeaponUpgrade.h"
#include <fstream>

Asteroids::Asteroids(int argc, char* argv[])
	: GameSession(argc, argv)


{
	for (int i = 0; i < MAX_HIGH_SCORES; i++)
	{
		mHighScores[i].score = 0;
		mHighScores[i].name = "";
	}
	mLevel = 0;
	mAsteroidCount = 0;
	mGameState = STATE_MENU;
	mSelectedMenuItem = MENU_START;
	mDifficultyEnabled = true;
	mHighScoreCount = 0;
	mCurrentScore = 0;
	mExtraLives = 0;
	mMenuAsteroidsCreated = false;
}

Asteroids::~Asteroids(void)
{
}

void Asteroids::Start()
{
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	mGameWorld->AddListener(thisPtr.get());
	mGameWindow->AddKeyboardListener(thisPtr);
	mGameWorld->AddListener(&mScoreKeeper);
	mScoreKeeper.AddListener(thisPtr);

	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");

	CreateGUI();
	LoadHighScores();

	mGameWorld->AddListener(&mPlayer);
	mPlayer.AddListener(thisPtr);

	ShowMenu();

	GameSession::Start();
}

void Asteroids::Stop()
{
	GameSession::Stop();
}

// =========================================================
// HELPER METHODS
// =========================================================

void Asteroids::HideAllInfoLabels()
{
	for (int i = 0; i < 8; i++)
		mInfoLabels[i]->SetVisible(false);
}

void Asteroids::HideMenuLabels()
{
	mTitleLabel->SetVisible(false);
	mSubtitleLabel->SetVisible(false);
	for (int i = 0; i < 4; i++)
		mMenuItemLabels[i]->SetVisible(false);
}

bool Asteroids::IsSpaceshipAlive()
{
	return mSpaceship && mSpaceship->GetWorld() != NULL;
}

void Asteroids::RemoveBackgroundAsteroids()
{
	for (uint i = 0; i < mBackgroundAsteroids.size(); i++)
	{
		shared_ptr<GameObject> obj = mBackgroundAsteroids[i].lock();
		if (obj && obj->GetWorld() != NULL)
			mGameWorld->FlagForRemoval(weak_ptr<GameObject>(obj));
	}
	mBackgroundAsteroids.clear();
	mMenuAsteroidsCreated = false;
}

void Asteroids::RemoveGameAsteroids()
{
	for (uint i = 0; i < mGameAsteroids.size(); i++)
	{
		shared_ptr<GameObject> obj = mGameAsteroids[i].lock();
		if (obj && obj->GetWorld() != NULL)
			mGameWorld->FlagForRemoval(weak_ptr<GameObject>(obj));
	}
	mGameAsteroids.clear();
}


void Asteroids::RemovePowerUps()
{
	for (uint i = 0; i < mPowerUpObjects.size(); i++)
	{
		shared_ptr<GameObject> obj = mPowerUpObjects[i].lock();
		if (obj && obj->GetWorld() != NULL)
			mGameWorld->FlagForRemoval(weak_ptr<GameObject>(obj));
	}
	mPowerUpObjects.clear();
}
// =========================================================
// MENU & STATE METHODS
// =========================================================

void Asteroids::ShowMenu()
{
	mGameState = STATE_MENU;

	if (!mMenuAsteroidsCreated)
	{
		CreateBackgroundAsteroids(5);
		mMenuAsteroidsCreated = true;
	}

	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mGameOverLabel->SetVisible(false);
	mPowerUpLabel->SetVisible(false);
	HideAllInfoLabels();

	mTitleLabel->SetVisible(true);
	mSubtitleLabel->SetVisible(true);
	UpdateMenuLabel();
}

void Asteroids::UpdateMenuLabel()
{
	std::string diffText = std::string("DIFFICULTY: ") + (mDifficultyEnabled ? "NORMAL" : "EASY");

	std::string texts[4] = {
		"START GAME",
		diffText,
		"INSTRUCTIONS",
		"HIGH SCORES"
	};

	for (int i = 0; i < 4; i++)
	{
		std::string text = (mSelectedMenuItem == i)
			? "> " + texts[i] + " <"
			: "  " + texts[i];
		mMenuItemLabels[i]->SetText(text);
		mMenuItemLabels[i]->SetVisible(true);
	}
}

void Asteroids::ShowInstructions()
{
	mGameState = STATE_INSTRUCTIONS;
	HideMenuLabels();
	HideAllInfoLabels();

	mInfoLabels[0]->SetText("== INSTRUCTIONS ==");
	mInfoLabels[1]->SetText("UP ARROW    - Thrust");
	mInfoLabels[2]->SetText("LEFT ARROW  - Rotate Left");
	mInfoLabels[3]->SetText("RIGHT ARROW - Rotate Right");
	mInfoLabels[4]->SetText("SPACE       - Shoot");
	mInfoLabels[5]->SetText("Collect power-ups to survive!");
	mInfoLabels[6]->SetText("");
	mInfoLabels[7]->SetText("Press ENTER to go back");
	for (int i = 0; i < 8; i++)
		mInfoLabels[i]->SetVisible(true);
}

void Asteroids::ShowHighScores()
{
	mGameState = STATE_HIGHSCORES;
	HideMenuLabels();
	HideAllInfoLabels();
	UpdateHighScoreLabel();
}

void Asteroids::UpdateHighScoreLabel()
{
	HideAllInfoLabels();
	mInfoLabels[0]->SetText("== HIGH SCORES ==");
	mInfoLabels[0]->SetVisible(true);

	if (mHighScoreCount == 0)
	{
		mInfoLabels[1]->SetText("No scores yet!");
		mInfoLabels[1]->SetVisible(true);
	}
	else
	{
		for (int i = 0; i < mHighScoreCount; i++)
		{
			std::ostringstream ss;
			ss << (i + 1) << ".  " << mHighScores[i].name
				<< "  -  " << mHighScores[i].score;
			mInfoLabels[i + 1]->SetText(ss.str());
			mInfoLabels[i + 1]->SetVisible(true);
		}
	}
	mInfoLabels[7]->SetText("Press ENTER to go back");
	mInfoLabels[7]->SetVisible(true);
}

void Asteroids::ShowEnterName()
{
	mGameState = STATE_ENTER_NAME;
	mEnteredName = "";
	HideMenuLabels();
	mScoreLabel->SetVisible(false);
	mLivesLabel->SetVisible(false);
	mGameOverLabel->SetVisible(false);
	mPowerUpLabel->SetVisible(false);
	mAsteroidCount = 0;

	HideAllInfoLabels();
	mInfoLabels[0]->SetText("== GAME OVER ==");
	mInfoLabels[1]->SetText("Your score: " + std::to_string(mCurrentScore));
	mInfoLabels[2]->SetText("Enter your name:");
	mInfoLabels[3]->SetText("_");
	mInfoLabels[4]->SetText("");
	mInfoLabels[5]->SetText("Press ENTER to confirm");
	for (int i = 0; i < 6; i++)
		mInfoLabels[i]->SetVisible(true);
}

void Asteroids::AddHighScore(const std::string& name, int score)
{
	if (mHighScoreCount < MAX_HIGH_SCORES)
	{
		mHighScores[mHighScoreCount].name = name;
		mHighScores[mHighScoreCount].score = score;
		mHighScoreCount++;
	}
	else
	{
		int minIdx = 0;
		for (int i = 1; i < MAX_HIGH_SCORES; i++)
			if (mHighScores[i].score < mHighScores[minIdx].score) minIdx = i;
		if (score > mHighScores[minIdx].score)
		{
			mHighScores[minIdx].name = name;
			mHighScores[minIdx].score = score;
		}
	}
	for (int i = 0; i < mHighScoreCount - 1; i++)
		for (int j = i + 1; j < mHighScoreCount; j++)
			if (mHighScores[j].score > mHighScores[i].score)
			{
				HighScoreEntry tmp = mHighScores[i];
				mHighScores[i] = mHighScores[j];
				mHighScores[j] = tmp;
			}
	SaveHighScores();
}

void Asteroids::SaveHighScores()
{
	std::ofstream file("scores.txt");
	if (!file.is_open()) return;
	file << mHighScoreCount << "\n";
	for (int i = 0; i < mHighScoreCount; i++)
		file << mHighScores[i].name << "\n" << mHighScores[i].score << "\n";
	file.close();
}

void Asteroids::LoadHighScores()
{
	std::ifstream file("scores.txt");
	if (!file.is_open()) return;
	file >> mHighScoreCount;
	if (mHighScoreCount > MAX_HIGH_SCORES) mHighScoreCount = MAX_HIGH_SCORES;
	for (int i = 0; i < mHighScoreCount; i++)
		file >> mHighScores[i].name >> mHighScores[i].score;
	file.close();
}

void Asteroids::StartGame()
{
	// Remove background asteroids,real asteroids and powersups before starting
	RemoveBackgroundAsteroids();
	RemoveGameAsteroids();
	RemovePowerUps();

	mGameState = STATE_PLAYING;
	mLevel = 0;
	mAsteroidCount = 0;
	mExtraLives = 0;
	mScoreKeeper.SetScoreEnabled(true);
	HideMenuLabels();
	HideAllInfoLabels();

	mPlayer.ResetLives();
	mScoreKeeper.ResetScore();
	mScoreLabel->SetText("Score: 0");
	mScoreLabel->SetText("Score: 0");
	mScoreLabel->SetVisible(true);
	mLivesLabel->SetText("Lives: 3");
	mLivesLabel->SetVisible(true);
	mPowerUpLabel->SetText("");
	mPowerUpLabel->SetVisible(true);

	mGameWorld->AddObject(CreateSpaceship());
	CreateAsteroids(10);

	if (mDifficultyEnabled)
		SetTimer(10000, SPAWN_POWERUPS);
}

// =========================================================
// POWERUP SPAWN METHODS
// =========================================================

void Asteroids::SpawnPowerUps()
{
	if (mGameState != STATE_PLAYING) return;
	if (!mDifficultyEnabled) return;

	int roll = rand() % 3;
	if (roll == 0) CreateExtraLife();
	else if (roll == 1) CreateInvulnerability();
	else CreateWeaponUpgrade();

	SetTimer(15000, SPAWN_POWERUPS);
}

void Asteroids::CreateExtraLife()
{
	shared_ptr<GameObject> powerup = make_shared<ExtraLife>();
	powerup->SetBoundingShape(make_shared<BoundingSphere>(powerup->GetThisPtr(), 4.0f));
	shared_ptr<Shape> shape = make_shared<Shape>("extralife.shape");
	powerup->SetShape(shape);
	powerup->SetScale(1.0f);
	mGameWorld->AddObject(powerup);
	mPowerUpObjects.push_back(weak_ptr<GameObject>(powerup));
}

void Asteroids::CreateInvulnerability()
{
	shared_ptr<GameObject> powerup = make_shared<Invulnerability>();
	powerup->SetBoundingShape(make_shared<BoundingSphere>(powerup->GetThisPtr(), 4.0f));
	shared_ptr<Shape> shape = make_shared<Shape>("invulnerability.shape");
	powerup->SetShape(shape);
	powerup->SetScale(1.0f);
	mGameWorld->AddObject(powerup);
	mPowerUpObjects.push_back(weak_ptr<GameObject>(powerup));
}

void Asteroids::CreateWeaponUpgrade()
{
	shared_ptr<GameObject> powerup = make_shared<WeaponUpgrade>();
	powerup->SetBoundingShape(make_shared<BoundingSphere>(powerup->GetThisPtr(), 4.0f));
	shared_ptr<Shape> shape = make_shared<Shape>("weaponupgrade.shape");
	powerup->SetShape(shape);
	powerup->SetScale(1.0f);
	mGameWorld->AddObject(powerup);
	mPowerUpObjects.push_back(weak_ptr<GameObject>(powerup));
}

// =========================================================
// KEYBOARD INPUT
// =========================================================

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (mGameState == STATE_MENU)
	{
		if (key == 13)
		{
			switch (mSelectedMenuItem)
			{
			case MENU_START:        StartGame();                                break;
			case MENU_DIFFICULTY:   mDifficultyEnabled = !mDifficultyEnabled;
				UpdateMenuLabel();                          break;
			case MENU_INSTRUCTIONS: ShowInstructions();                         break;
			case MENU_HIGHSCORES:   ShowHighScores();                           break;
			}
		}
		return;
	}

	if (mGameState == STATE_INSTRUCTIONS || mGameState == STATE_HIGHSCORES)
	{
		if (key == 13) ShowMenu();
		return;
	}

	if (mGameState == STATE_ENTER_NAME)
	{
		if (key == 13)
		{
			std::string finalName = mEnteredName.empty() ? "AAA" : mEnteredName;
			AddHighScore(finalName, mCurrentScore);
			mCurrentScore = 0;
			ShowMenu();
		}
		else if (key == 8)
		{
			if (!mEnteredName.empty()) mEnteredName.pop_back();
			mInfoLabels[3]->SetText(mEnteredName.empty() ? "_" : mEnteredName + "_");
		}
		else
		{
			if (mEnteredName.size() < 10)
			{
				mEnteredName += (char)toupper(key);
				mInfoLabels[3]->SetText(mEnteredName + "_");
			}
		}
		return;
	}

	if (mGameState == STATE_PLAYING)
	{
		if (key == ' ' && IsSpaceshipAlive())
			mSpaceship->Shoot();
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (mGameState == STATE_MENU)
	{
		if (key == GLUT_KEY_UP)
			mSelectedMenuItem = (mSelectedMenuItem - 1 + MENU_COUNT) % MENU_COUNT;
		else if (key == GLUT_KEY_DOWN)
			mSelectedMenuItem = (mSelectedMenuItem + 1) % MENU_COUNT;
		UpdateMenuLabel();
		return;
	}

	if (mGameState == STATE_PLAYING && IsSpaceshipAlive())
	{
		switch (key)
		{
		case GLUT_KEY_UP:    mSpaceship->Thrust(10); break;
		case GLUT_KEY_LEFT:  mSpaceship->Rotate(90); break;
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-90); break;
		default: break;
		}
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (mGameState == STATE_PLAYING && IsSpaceshipAlive())
	{
		switch (key)
		{
		case GLUT_KEY_UP:    mSpaceship->Thrust(0); break;
		case GLUT_KEY_LEFT:  mSpaceship->Rotate(0); break;
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
		default: break;
		}
	}
}

// =========================================================
// GAME WORLD EVENTS
// =========================================================

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		if (mGameState == STATE_PLAYING)
		{
			shared_ptr<GameObject> explosion = CreateExplosion();
			explosion->SetPosition(object->GetPosition());
			explosion->SetRotation(object->GetRotation());
			mGameWorld->AddObject(explosion);
		}
		if (mAsteroidCount > 0) mAsteroidCount--;
		if (mAsteroidCount <= 0 && mGameState == STATE_PLAYING)
			SetTimer(500, START_NEXT_LEVEL);
	}

	if (object->GetType() == GameObjectType("ExtraLife"))
	{
		if (mGameState != STATE_PLAYING) return;
		mExtraLives++;
		mPlayer.AddLife();
		std::ostringstream ss;
		ss << "Lives: " << (3 + mExtraLives);
		mLivesLabel->SetText(ss.str());
		mPowerUpLabel->SetText("+ EXTRA LIFE!");
		SetTimer(2000, 10);
	}

	if (object->GetType() == GameObjectType("Invulnerability"))
	{
		if (mGameState != STATE_PLAYING || !IsSpaceshipAlive()) return;
		mSpaceship->ActivateInvulnerability(5000);
		mPowerUpLabel->SetText("** INVULNERABLE! **");
		SetTimer(2000, 10);
	}

	if (object->GetType() == GameObjectType("WeaponUpgrade"))
	{
		if (mGameState != STATE_PLAYING || !IsSpaceshipAlive()) return;
		mSpaceship->ActivateWeaponUpgrade(10000);
		mPowerUpLabel->SetText("*** TRIPLE SHOT! ***");
		SetTimer(2000, 10);
	}
}

// =========================================================
// TIMER EVENTS
// =========================================================

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mGameWorld->AddObject(CreateSpaceship());
	}
	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
		if (mDifficultyEnabled)
			SetTimer(5000, SPAWN_POWERUPS);
	}
	if (value == SHOW_GAME_OVER)
	{
		ShowEnterName();
	}
	if (value == SPAWN_POWERUPS)
	{
		SpawnPowerUps();
	}
	if (value == 10)
	{
		mPowerUpLabel->SetText("");
	}
	if (value == 20)
	{
		mScoreKeeper.SetScoreEnabled(true);
	}
}

// =========================================================
// OBJECT CREATION
// =========================================================

shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);
	mSpaceship->Reset();
	return mSpaceship;
}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount += num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
		mGameAsteroids.push_back(weak_ptr<GameObject>(asteroid));
	}
}

void Asteroids::CreateBackgroundAsteroids(const uint num_asteroids)
{
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite =
			make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<BackgroundAsteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
		// Save weak pointer so we can remove them later
		mBackgroundAsteroids.push_back(weak_ptr<GameObject>(asteroid));
	}
}

void Asteroids::CreateGUI()
{
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));

	// Score label (top left)
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mScoreLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mScoreLabel), GLVector2f(0.0f, 1.0f));

	// Lives label (bottom left)
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	mLivesLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mLivesLabel), GLVector2f(0.0f, 0.0f));

	// Game over label
	mGameOverLabel = make_shared<GUILabel>("GAME OVER");
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	mGameOverLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mGameOverLabel), GLVector2f(0.5f, 0.5f));

	// Powerup notification label
	mPowerUpLabel = make_shared<GUILabel>("");
	mPowerUpLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mPowerUpLabel->SetVisible(false);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mPowerUpLabel), GLVector2f(0.5f, 0.75f));

	// Title
	mTitleLabel = make_shared<GUILabel>("=== SPACE DEFENDER ===");
	mTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mTitleLabel->SetVisible(true);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mTitleLabel), GLVector2f(0.5f, 0.88f));

	// Decorative line
	mSubtitleLabel = make_shared<GUILabel>("- - - - - - - - - - - - - - - - -");
	mSubtitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mSubtitleLabel->SetVisible(true);
	mGameDisplay->GetContainer()->AddComponent(
		static_pointer_cast<GUIComponent>(mSubtitleLabel), GLVector2f(0.5f, 0.80f));

	// Menu item labels
	const char* menuTexts[4] = {
		"START GAME",
		"DIFFICULTY: NORMAL",
		"INSTRUCTIONS",
		"HIGH SCORES"
	};
	float menuPositions[4] = { 0.65f, 0.55f, 0.45f, 0.35f };

	for (int i = 0; i < 4; i++)
	{
		mMenuItemLabels[i] = make_shared<GUILabel>(menuTexts[i]);
		mMenuItemLabels[i]->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		mMenuItemLabels[i]->SetVisible(true);
		mGameDisplay->GetContainer()->AddComponent(
			static_pointer_cast<GUIComponent>(mMenuItemLabels[i]),
			GLVector2f(0.5f, menuPositions[i]));
	}

	// Info labels
	float infoPositions[8] = { 0.78f, 0.68f, 0.60f, 0.52f, 0.44f, 0.36f, 0.28f, 0.18f };
	for (int i = 0; i < 8; i++)
	{
		mInfoLabels[i] = make_shared<GUILabel>("");
		mInfoLabels[i]->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		mInfoLabels[i]->SetVisible(false);
		mGameDisplay->GetContainer()->AddComponent(
			static_pointer_cast<GUIComponent>(mInfoLabels[i]),
			GLVector2f(0.5f, infoPositions[i]));
	}
}

void Asteroids::OnScoreChanged(int score)
{
	mCurrentScore = score;
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	mScoreLabel->SetText(msg_stream.str());
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	// Disable scoring briefly when player dies
	mScoreKeeper.SetScoreEnabled(false);
	SetTimer(100, 20);

	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	mLivesLabel->SetText(msg_stream.str());

	if (lives_left > 0)
		SetTimer(1000, CREATE_NEW_PLAYER);
	else
		SetTimer(500, SHOW_GAME_OVER);
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}