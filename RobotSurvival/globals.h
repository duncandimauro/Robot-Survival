#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>
#include <SDL.h>
#include "randomNumber.h"
#include <string>

using namespace std;

class Globals
{
public:

	//Static means that this variable is the same for ALL objects of type class Globals
	//Ex. if I had Globals global1   and   Globals global2 
    //and I went global1.PI = 11, then global2.PI would end up also equalling 11

	//Also - static stuff gets initialized and the start of the program
	
	//math helpers
	static const float PI;

	//useful for me as a dev
	static bool debugging;

	//SDL related
	static int ScreenWidth, ScreenHeight, ScreenScale;
	static SDL_Renderer* renderer; 

	static string clipOffDataHeader(string data);

};

#endif
