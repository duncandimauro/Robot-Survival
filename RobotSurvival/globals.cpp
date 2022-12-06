#include "globals.h"

//math helpers
const float Globals::PI = 3.14159;

//useful for me as a dev
bool Globals::debugging = false;

//SDL related
int Globals::ScreenWidth = 640, Globals::ScreenHeight = 352, Globals::ScreenScale = 2;
SDL_Renderer* Globals::renderer = NULL;

//clips off header. e.g. "clip: 50 114 44 49" turns into "50 114 44 49"
string Globals::clipOffDataHeader(string p_data)
{
	int pos = p_data.find(":", 0); //returns where we find the : in the string, otherwise return -1
	
	if (pos != -1)
		p_data = p_data.substr(pos + 1, p_data.length() - pos + 2);

	return p_data;

}