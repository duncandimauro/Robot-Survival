#ifndef WALL_H
#define WALL_H

#include "entity.h"

class Wall : public Entity
{
public:
	Wall(AnimationSet* p_animSet);

	void update();
	void changeAnimation(int newState, bool resetFrameToBeginning);
	void updateCollision() {} //doesn't do anything, walls don't move or get pushed in this game

};



#endif