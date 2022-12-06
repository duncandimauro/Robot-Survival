#include "game.h"

Game::Game()
{
	string resPath = getResourcePath();
	backgroundImage = loadTexture(resPath + "map.png", Globals::renderer);
	splashImage = loadTexture(resPath + "cyborgTitle.png", Globals::renderer);
	overlayImage = loadTexture(resPath + "overlay.png", Globals::renderer);

	splashShowing = true;
	overlayTimer = 2;

	//Load sounds
	SoundManager::soundManager.loadSound("hit", resPath + "Randomize2.wav");
	SoundManager::soundManager.loadSound("enemyHit", resPath + "Hit_Hurt9.wav");
	SoundManager::soundManager.loadSound("swing", resPath + "Hit_Hurt3.wav");
	SoundManager::soundManager.loadSound("dash", resPath + "dash.wav");
	SoundManager::soundManager.loadSound("growl", resPath + "Randomize34.wav");
	SoundManager::soundManager.loadSound("enemyDie", resPath + "Randomize41.wav");

	song = Mix_LoadMUS(string(resPath + "Fatal Theory.wav").c_str());
	if (song != NULL)
		Mix_PlayMusic(song, -1);

	//holds a list of group types. This list describes the types of groups of data our frames can have
	list<DataGroupType> dataGroupTypes;

	//so what data can a frame have? clip, offset, duration, and index all default
	
	//collisionBoxes (although we have hardcoded the collision boxes - they don't change frame by frame)
	DataGroupType colBoxType;
	colBoxType.groupName = "collisionBox";
	colBoxType.dataType = DataGroupType::DATATYPE_BOX;

	//hitBoxes
	DataGroupType hitBoxType;
	hitBoxType.groupName = "hitBox";
	hitBoxType.dataType = DataGroupType::DATATYPE_BOX;

	//damage
	DataGroupType dmgType;
	dmgType.groupName = "damage";
	dmgType.dataType = DataGroupType::DATATYPE_NUMBER;

	//add all of these dataTypes to the list
	dataGroupTypes.push_back(colBoxType);
	dataGroupTypes.push_back(hitBoxType);
	dataGroupTypes.push_back(dmgType);

	heroAnimSet = new AnimationSet();
	heroAnimSet->loadAnimationSet("hero.fdset", dataGroupTypes, true, 0, true);

	wallAnimSet = new AnimationSet();
	wallAnimSet->loadAnimationSet("wall.fdset", dataGroupTypes); //default - don't set color key, don't need white texture

	glob1AnimSet = new AnimationSet();
	glob1AnimSet->loadAnimationSet("glob1.fdset", dataGroupTypes, true, 0, true);

	glob2AnimSet = new AnimationSet();
	glob2AnimSet->loadAnimationSet("glob2.fdset", dataGroupTypes, true, 0, true);

	//CREATING THE HERO - note, x and y denote the center of the entity
	hero = new Hero(heroAnimSet);
	hero->invincibleTimer = 0; //maybe can eliminate this?
	hero->x = Globals::ScreenWidth / 2; //this too???
	hero->y = Globals::ScreenHeight / 2;//this too?

	//tells keyboard input to manage hero
	heroInput.hero = hero;

	//add hero to the entity list
	Entity::entities.push_back(hero);


	int tileSize = 32;
	
	//BUILD ALL WALLS
	
	//first build walls on top and bottom of screen
	for (int i = 0; i < Globals::ScreenWidth / tileSize; i++) //since walls are 32 x 32 and the screen width is divisible by 32
	{
		//fills in top row
		Wall* newWall = new Wall(wallAnimSet);
		newWall->x = i * tileSize + tileSize/2;
		newWall->y = tileSize / 2;
		walls.push_back(newWall);
		Entity::entities.push_back(newWall);

		//re-using pointer to create bottom row
		newWall = new Wall(wallAnimSet);
		newWall->x = i * tileSize + tileSize / 2;
		newWall->y = Globals::ScreenHeight - tileSize / 2;
		walls.push_back(newWall);
		Entity::entities.push_back(newWall);
	}
	//now the sides - start at i = 1 since we already have walls in the corners
	for (int i = 1; i < Globals::ScreenHeight / tileSize - 1; i++)
	{
		//fills in left column
		Wall* newWall = new Wall(wallAnimSet);
		newWall->x = tileSize / 2;
		newWall->y = i * tileSize + tileSize / 2;
		walls.push_back(newWall);
		Entity::entities.push_back(newWall);

		//re-using pointer to create right column
		newWall = new Wall(wallAnimSet);
		newWall->x = Globals::ScreenWidth - tileSize / 2;
		newWall->y = i * tileSize + tileSize / 2;
		walls.push_back(newWall);
		Entity::entities.push_back(newWall);
	}

}

Game::~Game()
{
	cleanup(backgroundImage);
	cleanup(splashImage);
	cleanup(overlayImage);

	Mix_PauseMusic;
	Mix_FreeMusic(song);

	if (scoreTexture != NULL)
		cleanup(scoreTexture);

	Entity::removeAllFromList(&Entity::entities, false); //don't delete them, just remove from list

	delete heroAnimSet;
	delete wallAnimSet;
	delete glob1AnimSet;
	delete glob2AnimSet;

	delete hero;

	Entity::removeAllFromList(&walls, true); //delete all the walls
	Entity::removeAllFromList(&enemies, true); //delete all the enemies
}

void Game::update()
{
	bool quit = false;

	//enemy related
	int enemiesToBuild = 2;
	int enemiesBuilt = 0;
	float enemyBuildTimer = 1;

	SDL_Event e;
	//setup my time controller before the game starts
	TimeController::timeController.reset();

	while (!quit)
	{
		TimeController::timeController.updateTime(); //can use this to freeze time if u want, don't update time

		Entity::removeInactiveEntitiesFromList(&Entity::entities, false); //removes them from list, does not delete

		//remove/delete the enemies in the enemy list who are dead/inactive
		Entity::removeInactiveEntitiesFromList(&enemies, true);

		//check for any events that might have happened
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) //happens when you close the window
				quit = true;

			//if keydown event (stuff we haven't already defined in the hero class)
			if (e.type == SDL_KEYDOWN)
			{
				//switch case on which button was pressed
				switch (e.key.keysym.scancode)
				{
				case SDL_SCANCODE_ESCAPE: //esc key
					quit = true;
					break;
				case SDL_SCANCODE_SPACE:
					if (splashShowing)
						splashShowing = false;
					if (overlayTimer <= 0 && hero->hp < 1)
					{ //cleanup and restart game
						enemiesToBuild = 2;
						enemiesBuilt = 0;
						enemyBuildTimer = 3;
						overlayTimer = 2;

						Glob::globsKilled = 0;
						if (scoreTexture != NULL)
						{
							cleanup(scoreTexture);
							scoreTexture = NULL;
						}
					}

					//remove all existing enemies (range-based for loop)
					//other option is for (list<Entity*>::iterator enemy = enemies.begin(); enemy != enemies.end(); enemy++)
					for (auto enemy : enemies)
					{
						enemy->active = false;
					}

					hero->revive();
					break;
				}
			}
			heroInput.update(&e); //send the input to the hero class
		}
		//make our overlay timer tick down
		if (hero->hp < 1 && overlayTimer > 0)
		{
			overlayTimer -= TimeController::timeController.dT;
		}

		//update all entities after checking input
		for (list<Entity*>::iterator entity = Entity::entities.begin(); entity != Entity::entities.end(); entity++)
		{
			//update all entities in game world at once
			(*entity)->update();
		}

		//spawn enemies
		if (hero->hp > 0 && !splashShowing)
		{
			if (enemiesToBuild == enemiesBuilt)
			{
				enemiesToBuild = enemiesToBuild * 2;
				enemiesBuilt = 0;
				enemyBuildTimer = 4;
			}
			enemyBuildTimer -= TimeController::timeController.dT;
			if (enemyBuildTimer <= 0 && enemiesBuilt < enemiesToBuild && enemies.size() < 10)
			{
				int val = rand() % 10; //0-9

				Glob* enemy = NULL;

				if (val > 2)
				{
					enemy = new Glob(glob1AnimSet, Glob::BLUE);
				}
				else
				{
					enemy = new Glob(glob2AnimSet, Glob::RED);
				}

				//set enemy's position to somewhere random within the arena's open space
				enemy->x = getRandomNumber(Globals::ScreenWidth - (2 * 32) - 32) + 32 + 16;
				enemy->y = getRandomNumber(Globals::ScreenHeight - (2 * 32) - 32) + 32 + 16;
				enemy->invincibleTimer = 0.1;

				enemies.push_back(enemy);
				Entity::entities.push_back(enemy);
			}
		}

		//draw all entities
		draw();
	}
}

void Game::draw()
{
	//clear the screen
	SDL_SetRenderDrawColor(Globals::renderer, 145, 133, 129, SDL_ALPHA_OPAQUE); //renderer set to grey
	SDL_RenderClear(Globals::renderer); //clear the screen with the above color

	if (splashShowing)
	{
		renderTexture(splashImage, Globals::renderer, 0, 0);
	}
	else
	{
		//draw background
		renderTexture(backgroundImage, Globals::renderer, 0, 0);

		//sort all entities based on y-depth
		Entity::entities.sort(Entity::EntityCompare); //give it a function to sort things, aka the EntityCompare function

		//draw all of the entities - draws to the renderer
		for (list<Entity*>::iterator entity = Entity::entities.begin(); entity != Entity::entities.end(); entity++)
		{
			(*entity)->draw();
		}

		if (overlayTimer <= 0 && hero->hp < 1)
		{
			renderTexture(overlayImage, Globals::renderer, 0, 0);
			if (scoreTexture == NULL)
			{
				//generate the score text
				SDL_Color color = { 255, 255, 255, 255 }; //white

				stringstream ss;
				ss << "Enemies terminated: " << Glob::globsKilled;

				string resPath = getResourcePath();
				scoreTexture = renderText(ss.str(), resPath + "vermin_vibes_1989.ttf", color, 30, Globals::renderer);
			}
			renderTexture(scoreTexture, Globals::renderer, 20, 50);
		}
	}
	//after done drawing/rendering, show it to the screen
	SDL_RenderPresent(Globals::renderer);
}