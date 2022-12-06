#include "cleanup.h"
#include "res_path.h"
#include "drawing_functions.h" //includes SDL.h , SDL_image.h, and SDL_ttf.h, iostream, namespace std, res_path.h
#include <SDL_mixer.h>
#include "globals.h"
#include "game.h"
#include <cstdlib> //srand, rand
#include <ctime> //time
#include <Windows.h>

int main(int argc, char* args[])
{

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);

	//set the random seed for the random number sequence - same seed results in same rand num sequence
	srand(time(0)); //using time(0) results in different seed every time (based on computer time I think?)

	//setup SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		cout << "error initializing SDL. Error: " << SDL_GetError() << endl;
		return 1;
	}

	
	//setup window
	SDL_Window* window = SDL_CreateWindow("Robot Survival", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		Globals::ScreenWidth * Globals::ScreenScale, Globals::ScreenHeight * Globals::ScreenScale,
		SDL_WINDOW_SHOWN); //for fullscreen mode: SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN

	if (window == nullptr)
	{
		cout << "window error: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	
	//setup renderer
	Globals::renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (Globals::renderer == nullptr)
	{
		cout << "renderer error: " << SDL_GetError() << endl;
		cleanup(window);
		SDL_Quit();
		return 1;
	}
	//this is the size to draw things at before we scale it to the screen size dimensions mentioned in createWindow
	SDL_RenderSetLogicalSize(Globals::renderer, Globals::ScreenWidth, Globals::ScreenHeight);

	//initialize SDL_image
	if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		cout << "initializing img_init png error: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	//initialize text to font
	if (TTF_Init() != 0)
	{
		cout << "sdl ttf did not initialze: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	//initialize sdl mixer - amt of channels is how many sounds can play at once
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		cout << "mixer did not initialize: " << SDL_GetError() << endl;
		SDL_Quit();
		return 1;
	}

	//load up a texture to draw - from will usher tutorial
	/*string resPath = getResourcePath();
	
	SDL_Texture* texture = loadTexture(resPath + "map.png", Globals::renderer);

	//run game for 5000 ticks (5000ms)

	while (SDL_GetTicks() < 5000)
	{

		//BASIC DRAWING PROCEDURE:
		
		//clear the screen
		SDL_RenderClear(Globals::renderer);
		//draw what we want to the renderer
		renderTexture(texture, Globals::renderer, 0, 0);
		//show image we've been rendering
		SDL_RenderPresent(Globals::renderer);
	}
	*/
	
	Game game;
	system("CLS");
	game.update();

	cleanup(Globals::renderer);
	cleanup(window);
	//cleanup(texture);

	SDL_Quit();

	return 0;
}