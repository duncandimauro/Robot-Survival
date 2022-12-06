#include "animation.h"

Animation::Animation(string p_name)
	: name(p_name) {}

int Animation::getNextFrameIndex(int p_frameIndex)
{
	if (p_frameIndex + 1 < frames.size())
		return p_frameIndex + 1;
	else
		return 0;
}

Frame* Animation::getNextFrame(Frame* p_frame)
{
	return getFrame(getNextFrameIndex(p_frame->frameIndex));
}

int Animation::getEndFrameIndex()
{
	return frames.size() - 1;
}

Frame* Animation::getFrame(int p_frameIndex)
{
	//MY OWN CODE
	//if (frames.size() == 0)
	//	return nullptr;

	//for (list<Frame>::iterator f = frames.begin(); f != frames.end(); f++)
	//{
	//	if (p_frameIndex == f->frameIndex || f.frameIndex == frames.size() - 1)
	//	{
	//		return &(*f);
	//	}
	//}

	//This way might work? idrk MY OWN OTHER CODE, must have &f, &&f also works, just f breaks it
	for (auto &f : frames)
	{
		if (p_frameIndex == f.frameIndex || f.frameIndex == frames.size() - 1)
		{
			return &f;
		}
	}
}

void Animation::loadAnimation(ifstream& file, list<DataGroupType>& groupTypes) 
{
	//We're at the point in the file that says "animations"

	//GET NUM OF FRAMES
	getline(file, name);
	string buffer;
	getline(file, buffer);
	stringstream ss;
	buffer = Globals::clipOffDataHeader(buffer);
	ss << buffer;
	int numberOfFrames;
	ss >> numberOfFrames;

	//Loads each of the frames which have data concerning clip, offset, duration, and index
	//Pushes back each of these frame objects into a list titled "frames"
	for (int i = 0; i < numberOfFrames; i++) 
	{
		Frame newFrame;
		newFrame.loadFrame(file, groupTypes);
		frames.push_back(newFrame);
	}


}