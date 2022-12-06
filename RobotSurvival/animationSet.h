#ifndef ANIMATIONSET_H
#define ANIMATIONSET_H

#include "animation.h"

using namespace std;

class AnimationSet
{
public:
	string imageName;
	SDL_Texture* spriteSheet; //holds the spritesheet image for all of our animations
	SDL_Texture* whiteSpriteSheet = NULL; //I use this spritesheet to show damage, entities flash white
	list<Animation> animations;

	~AnimationSet(); //properly cleans up any dynamic memory e.g. textures

	Animation* getAnimation(string name);

	//loads a fdset file, also takes a list of what types of data we expect to loading.
	//Also, if we're working with an 8bit image and we know in that 8bit image what coloured pixel in the palette 
	//we would prefer to be transparent, then we can say yes to 'colorKeying'(make a colour in the palette actually equal full transparent)
	//and then just use the index of that transparentPixel (e.g, if its the third colour in the palette, then put in 2 as index starts at 0)
	//if you need an alternative white version of the sprite sheet, then make this last option true (maybe move this out of this class? not sure)
	void loadAnimationSet(string fileName, list<DataGroupType>& groupTypes, bool setColorKey = false, int transparentPixelIndex = 0, bool createWhiteTexture = false);
};

#endif