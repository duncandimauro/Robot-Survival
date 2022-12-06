#ifndef GAME_H
#define GAME_H

#include "globals.h"
#include "Hero.h"
#include "glob.h"
#include "wall.h"
#include "keyboardInput.h"
#include "drawing_functions.h"
#include "soundManager.h"


class Game
{
public:
	Mix_Music* song;

	AnimationSet* heroAnimSet;
	AnimationSet* glob1AnimSet;
	AnimationSet* glob2AnimSet;
	AnimationSet* wallAnimSet;

	SDL_Texture* backgroundImage;
	SDL_Texture* splashImage;
	SDL_Texture* overlayImage;

	SDL_Texture* scoreTexture = NULL; //for drawing strings to screen (ie the score)

	Hero* hero;
	KeyboardInput heroInput;

	list <Entity*> enemies;
	list<Entity*> walls;

	bool splashShowing;
	float overlayTimer;

	Game();
	~Game();

	void update();
	void draw();


};



#endif