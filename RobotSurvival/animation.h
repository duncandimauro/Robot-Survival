#ifndef ANIMATION_H
#define ANIMATION_H

#include "frame.h"

class Animation
{
public:
	string name; //name of the animation
	list<Frame> frames; //list of our frames

	Animation(string name = "");

	int getNextFrameIndex(int frameIndex); //returns the next frame number in the list
	Frame* getNextFrame(Frame* frame);
	int getEndFrameIndex(); //returns the last frame's number
	Frame* getFrame(int frameIndex); //get frame using frame number

	void loadAnimation(ifstream& file, list<DataGroupType> &groupTypes);
};

#endif