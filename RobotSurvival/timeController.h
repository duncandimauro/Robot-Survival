#ifndef TIMECONTROLLER_H
#define TIMECONTROLLER_H
#include <SDL.h>

//FOR PROPER PROGRAMMING
//https://stackoverflow.com/questions/15572665/c-structs-with-member-functions-vs-classes-with-public-variables

class TimeController
{
public:
	static const int PLAY_STATE, PAUSE_STATE;
	
	//make getter for all these public var?
	int timeState;
	Uint32 lastUpdate; //the last time we checked how many ticks we were up to
	float dT; //delta time, in seconds (1 = 1 second) Time since the last frame was rendered onto screen

	TimeController();
	void updateTime(); //update lastUpdate and dT
	void pause();
	void resume();
	void reset();

	static TimeController timeController;
};

#endif