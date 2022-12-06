#include "frame.h"

void Frame::Draw(SDL_Texture* spriteSheet, float x, float y)
{
	SDL_Rect dest; //destination of where we want to draw this frame
	dest.x = x - offSet.x;
	dest.y = y - offSet.y;
	dest.w = clip.w;
	dest.h = clip.h;

	renderTexture(spriteSheet, Globals::renderer, dest, &clip);
}

void Frame::loadFrame(ifstream& file, list<DataGroupType>& groupTypes) 
{
	//Ok, we dragged these groupTypes allllll the way here, so we could construct groups on our frame to hold the data in
	//Ex, build collision box group, hitbox group, damage group
	GroupBuilder::buildGroups(groupTypes, frameData);

	string buffer;
	//frame - dud data saying this is a frame - Line in the .fdset file which just says "line"
	getline(file, buffer);
	
	//clip
	getline(file, buffer);
	
	stringstream ss; //helpful for building and reading strings
	
	buffer = Globals::clipOffDataHeader(buffer); //get the string after the ":" in this line
	
	ss << buffer; //put the buffer into the string stream
	
	ss >> clip.x >> clip.y >> clip.w >> clip.h; //puts each of the 4 numbers into their respective x, y, w, and h
	                                            //this works since it reads until a white space or until a new line
	
	//USE SAME METHOD WITH OTHER SECTIONS
	
	//offset/pivot
	getline(file, buffer);
	ss.clear();
	buffer = Globals::clipOffDataHeader(buffer);
	ss << buffer;
	ss >> offSet.x >> offSet.y;
	//duration
	getline(file, buffer);
	ss.clear();
	buffer = Globals::clipOffDataHeader(buffer);
	ss << buffer;
	ss >> duration;
	//index
	getline(file, buffer);
	ss.clear();
	buffer = Globals::clipOffDataHeader(buffer);
	ss << buffer;
	ss >> frameIndex;

	GroupBuilder::loadGroups(file, frameData);
}