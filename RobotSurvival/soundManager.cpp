#include "soundManager.h"

SoundManager::SoundManager() //not needed?
{
}

SoundManager::~SoundManager()
{
	for (soundListing sl : sounds)
	{
		Mix_FreeChunk(sl.sound);
	}
}

void SoundManager::loadSound(string name, string file)
{
	soundListing listing;
	listing.name = name;
	listing.sound = Mix_LoadWAV(file.c_str());

	sounds.push_back(listing);
}

void SoundManager::playSound(string name)
{
	for (soundListing sl : sounds)
	{
		if (sl.name == name)
		{
			Mix_PlayChannel(-1, sl.sound, 0);
			break;
		}
	}
}

SoundManager SoundManager::soundManager;


