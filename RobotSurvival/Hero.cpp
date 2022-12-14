#include "Hero.h"

//reference variables
const string Hero::HERO_ANIM_UP = "up";
const string Hero::HERO_ANIM_DOWN = "down";
const string Hero::HERO_ANIM_LEFT = "left";
const string Hero::HERO_ANIM_RIGHT = "right";

const string Hero::HERO_ANIM_IDLE_UP = "idleUp";
const string Hero::HERO_ANIM_IDLE_DOWN = "idleDown";
const string Hero::HERO_ANIM_IDLE_LEFT = "idleLeft";
const string Hero::HERO_ANIM_IDLE_RIGHT = "idleRight";

const string Hero::HERO_ANIM_SLASH_UP = "slashUp";
const string Hero::HERO_ANIM_SLASH_DOWN = "slashDown";
const string Hero::HERO_ANIM_SLASH_LEFT = "slashLeft";
const string Hero::HERO_ANIM_SLASH_RIGHT = "slashRight";

const string Hero::HERO_ANIM_DASH_UP = "dashUp";
const string Hero::HERO_ANIM_DASH_DOWN = "dashDown";
const string Hero::HERO_ANIM_DASH_LEFT = "dashLeft";
const string Hero::HERO_ANIM_DASH_RIGHT = "dashRight";

const string Hero::HERO_ANIM_DIE = "die";

const int Hero::HERO_STATE_IDLE = 0;
const int Hero::HERO_STATE_MOVE = 1;
const int Hero::HERO_STATE_SLASH = 2;
const int Hero::HERO_STATE_DASH = 3;
const int Hero::HERO_STATE_DEAD = 4;

Hero::Hero(AnimationSet* p_animSet)
{
	animSet = p_animSet;
	type = "hero";

	//setup default hero values
	x = Globals::ScreenWidth / 2;
	y = Globals::ScreenHeight / 2;
	moveSpeed = 0;
	moveSpeedMax = 50;
	hp = hpMax = 20;
	damage = 0;
	collisionBoxW = 20; //just play around until you get a good collision box width and height
	collisionBoxH = 24;
	collisionBoxYOffset = -20; //this collision box will be drawn 20 up from your x and y?

	direction = DIR_DOWN; //so the hero faces the player

	changeAnimation(HERO_STATE_IDLE, true);

	updateCollisionBox();
}

void Hero::update()
{
	//check if dead
	if (hp < 1 && state != HERO_STATE_DEAD)
	{
		changeAnimation(HERO_STATE_DEAD, true);
		moving = false;
		die();
	}

	//maybe could put ALL THIS STUFF BELOW into a function with living Entity or something?

	//look at 11:20 if I want to refresh on this order of stuff
	updateCollisionBox();
	updateMovement();
	updateCollisions();

	updateHitBox();
	updateDamages();

	updateAnimation();
	updateInvincibleTimer();
}

void Hero::slash()
{
	if (hp > 0 && (state == HERO_STATE_MOVE || state == HERO_STATE_IDLE))
	{
		moving = false;
		frameTimer = 0;
		changeAnimation(HERO_STATE_SLASH, true);
		SoundManager::soundManager.playSound("swing");
	}
}

void Hero::dash()
{
	if (hp > 0 && (state == HERO_STATE_MOVE || state == HERO_STATE_IDLE))
	{
		moving = false;
		frameTimer = 0;

		//push the hero in the direction they are travelling
		slideAngle = angle;
		slideAmount = 300; //just play around until you like this number
		invincibleTimer = 0.1; //invincible for 0.1 seconds

		changeAnimation(HERO_STATE_DASH, true);
		SoundManager::soundManager.playSound("dash");
	}
}

void Hero::die()
{
	moving = false;
	//state = HERO_STATE_DEAD;
	changeAnimation(HERO_STATE_DEAD, true); //change animation already changes the state
}


void Hero::revive()
{
	hp = hpMax;
	changeAnimation(HERO_STATE_IDLE, true);
	moving = false;
	x = Globals::ScreenWidth / 2;
	y = Globals::ScreenHeight / 2;
	slideAmount = 0;
}


void Hero::changeAnimation(int newState, bool resetFrameToBeginning)
{
	state = newState;

	if (state == HERO_STATE_IDLE)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(HERO_ANIM_IDLE_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(HERO_ANIM_IDLE_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(HERO_ANIM_IDLE_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(HERO_ANIM_IDLE_RIGHT);
	}
	else if (state == HERO_STATE_MOVE)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(HERO_ANIM_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(HERO_ANIM_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(HERO_ANIM_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(HERO_ANIM_RIGHT);
	}
	else if (state == HERO_STATE_SLASH)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(HERO_ANIM_SLASH_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(HERO_ANIM_SLASH_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(HERO_ANIM_SLASH_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(HERO_ANIM_SLASH_RIGHT);
	}
	else if (state == HERO_STATE_DASH)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(HERO_ANIM_DASH_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(HERO_ANIM_DASH_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(HERO_ANIM_DASH_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(HERO_ANIM_DASH_RIGHT);
	}
	else if (state == HERO_STATE_DEAD)
	{
		currentAnim = animSet->getAnimation(HERO_ANIM_DIE);
	}

	if (resetFrameToBeginning)
		currentFrame = currentAnim->getFrame(0);
	else
		currentFrame = currentAnim->getFrame(currentFrame->frameIndex);
}

void Hero::updateAnimation()
{

	if (currentFrame == NULL || currentAnim == NULL)
		return; //can't do much with animations without pointers pointing to them

	//if state says moving, but we're not, then change state/animation to idle
	if (state == HERO_STATE_MOVE && !moving)
	{
		changeAnimation(HERO_STATE_IDLE, true);
	}
	//if should be showing running animation, lets change state properly
	if (state != HERO_STATE_MOVE && moving)
	{
		changeAnimation(HERO_STATE_MOVE, true);
	}

	frameTimer += TimeController::timeController.dT;
	//time to change frames 

	if (frameTimer >= currentFrame->duration)
	{
		//if we're at the end of an animation - getNextFrame automatically returns to 0 if we are at the
		//end of an animation, but for the hero, certain things change the state at the end of the animation
		//sometimes so we made sure to check just in case

		if (currentFrame->frameIndex == currentAnim->getEndFrameIndex())
		{
			if (state == HERO_STATE_SLASH || state == HERO_STATE_DASH)
			{
				//change back to moving state/anim
				changeAnimation(HERO_STATE_MOVE, true);
			}
			else if (state == HERO_STATE_DEAD && hp > 0)
			{
				//was dead, but now has more hp, get back up (move state)
				changeAnimation(HERO_STATE_MOVE, true);
			}
			else
			{
				//just reset animation (loops it back around)
				currentFrame = currentAnim->getFrame(0);
			}
		}
		else
		{
			//just move onto the next frame in this animation
			currentFrame = currentAnim->getNextFrame(currentFrame);
		}
		frameTimer = 0;
	}
}

void Hero::updateDamages()
{
	if (active && hp > 0 && invincibleTimer <= 0)
	{
		for (auto entity = Entity::entities.begin(); entity != Entity::entities.end(); entity++)
		{
			//if you have bullets and stuff, make sure you check against those too
			if ((*entity)->active && (*entity)->type == "enemy")
			{
				//we know enemies are living entities, so cast it to that
				LivingEntity* enemy = (LivingEntity*)(*entity);

				if (enemy->damage > 0 && Entity::checkCollision(collisionBox, enemy->hitBox))
				{
					hp -= enemy->damage;

					//still alive
					if (hp > 0)
					{
						invincibleTimer = 0.3;
						SoundManager::soundManager.playSound("hit");
					}

					slideAngle = Entity::angleBetweenTwoEntities((*entity), this);
					slideAmount = 200;
				}
			}
		}
	}
}