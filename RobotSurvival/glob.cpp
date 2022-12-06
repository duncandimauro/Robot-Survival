#include "glob.h"

const int Glob::BLUE = 1;
const int Glob::RED = 2;

const string Glob::GLOB_ANIM_UP = "up";
const string Glob::GLOB_ANIM_DOWN = "down";
const string Glob::GLOB_ANIM_LEFT = "left";
const string Glob::GLOB_ANIM_RIGHT = "right";

const string Glob::GLOB_ANIM_IDLE_UP = "idleUp";
const string Glob::GLOB_ANIM_IDLE_DOWN = "idleDown";
const string Glob::GLOB_ANIM_IDLE_LEFT = "idleLeft";
const string Glob::GLOB_ANIM_IDLE_RIGHT = "idleRight";

const string Glob::GLOB_ANIM_ATTACK_UP = "attackUp";
const string Glob::GLOB_ANIM_ATTACK_DOWN = "attackDown";
const string Glob::GLOB_ANIM_ATTACK_LEFT = "attackLeft";
const string Glob::GLOB_ANIM_ATTACK_RIGHT = "attackRight";

const string Glob::GLOB_ANIM_TELEGRAPH_UP = "telegraphUp";
const string Glob::GLOB_ANIM_TELEGRAPH_DOWN = "telegraphDown";
const string Glob::GLOB_ANIM_TELEGRAPH_LEFT = "telegraphLeft";
const string Glob::GLOB_ANIM_TELEGRAPH_RIGHT = "telegraphRight";

const string Glob::GLOB_ANIM_DIE = "die";

const int Glob::GLOB_STATE_IDLE = 0;
const int Glob::GLOB_STATE_MOVE = 1;
const int Glob::GLOB_STATE_ATTACK = 2;
const int Glob::GLOB_STATE_TELEGRAPH = 3;
const int Glob::GLOB_STATE_DEAD = 4;

const int Glob::GLOB_AI_NORMAL = 0;
const int Glob::GLOB_AI_CHASE = 1;

int Glob::globsKilled = 0;


Glob::Glob(AnimationSet* p_animSet, int GlobType)
{
	animSet = p_animSet;

	type = "enemy";

	//defaults
	x = Globals::ScreenWidth / 2;
	y = Globals::ScreenHeight / 2;
	moveSpeed = 0;
	switch (GlobType)
	{
	case BLUE:
		moveSpeedMax = 20;
		Glob::GlobType = BLUE;
		break;
	case RED:
		moveSpeedMax = 10;
		Glob::GlobType = RED;
		break;
	default:
		break;
	}

	hp = hpMax = 10 + (rand() % 20); //10 - 29
	damage = 0;
	collisionBoxW = 18;
	collisionBoxH = 20;
	collisionBox.w = collisionBoxW;
	collisionBox.h = collisionBoxH;

	collisionBoxYOffset = -14;

	direction = DIR_DOWN;
	changeAnimation(GLOB_STATE_IDLE, true);

	updateCollisionBox();
}

void Glob::updateHitBox() 
{
	if (GlobType == BLUE) //Blue behaves normally
	{
		LivingEntity::updateHitBox();
	}
	else if (GlobType == RED) //Red is a constantly walking hitbox
	{
		hitBox.w = collisionBox.w + 2;
		hitBox.h = collisionBox.h + 2;
		hitBox.x = collisionBox.x - 1;
		hitBox.y = collisionBox.y - 1;
		if (hp > 0)
			damage = 5;
		else
			damage = 0;
	}
}

void Glob::update()
{
	//check if died
	if (hp < 1 && state != GLOB_STATE_DEAD)
	{
		changeAnimation(GLOB_STATE_DEAD, true);
		moving = false;
		die();
	}

	think();

	updateCollisionBox();
	updateMovement();
	updateCollisions();
	updateHitBox();
	updateDamages();

	updateAnimation();
	updateInvincibleTimer();
}

void Glob::think()
{
	if (state == GLOB_STATE_IDLE || state == GLOB_STATE_MOVE)
	{
		thinkTimer -= TimeController::timeController.dT;

		//time to choose an action
		if (thinkTimer <= 0)
		{
			//reset the timer
			if (GlobType == BLUE)
			{
				thinkTimer = rand() % 5; //0 to 5 seconds
			}
			else if (GlobType == RED)
			{
				thinkTimer = 0.5; //.5 sec
			}
			
			int action = rand() % 10; //0-9

			if (action < 3) //be still
			{
				moving = false;
				aiState = GLOB_AI_NORMAL;
				changeAnimation(GLOB_STATE_IDLE, true);
			}
			else if (GlobType == RED) //Red globs just hang out and move randomly
			{
				angle = rand() % 360; //random angle to move in
				moving = true;
				changeAnimation(GLOB_STATE_MOVE, state != GLOB_STATE_MOVE);

				move(angle);
			}
			else if (GlobType == BLUE) //Blue globs chase you and attack
			{
				findNearestTarget();

				//found a target and its alive, let's get em
				if (target != nullptr && target->hp > 0)
				{
					float dist = Entity::distanceBetweenTwoEntities(this, target);

					float attackRange = 100;

					//if in range
					if (dist < attackRange)
					{
						telegraph(); //telegraph attack first so players have a chance to dodge
						aiState = GLOB_AI_NORMAL;
					}
					else
					{
						//otherwise move up to the player/target
						aiState = GLOB_AI_CHASE;
						moving = true;
						changeAnimation(GLOB_STATE_MOVE, state != GLOB_STATE_MOVE);
					}
				}
				else
				{
					//no targets, go idle
					moving = false;
					aiState = GLOB_AI_NORMAL;
					changeAnimation(GLOB_STATE_IDLE, true);
				}
			}
		}
	}
	//if chasing a target, then do that
	if (aiState == GLOB_AI_CHASE && hp > 0 && active)
	{
		//get the angle between me and the target
		angle = Entity::angleBetweenTwoEntities(this, target);
		//move that way
		move(angle);
	}
}

void Glob::telegraph() 
{
	moving = false;
	frameTimer = 0;
	changeAnimation(GLOB_STATE_TELEGRAPH, true);
}

void Glob::attack()
{
	moving = false;
	frameTimer = 0;
	slideAmount = 100; //glob gets launched towards player
	//uncomment if you want the angle to update at time of attack as opposed to telegraph
	//angle = Entity::angleBetweenTwoEntities(this, target);
	slideAngle = angle;
	changeAnimation(GLOB_STATE_ATTACK, true);
}

void Glob::die()
{
	moving = false;
	state = GLOB_STATE_DEAD;
	changeAnimation(state, true);
	SoundManager::soundManager.playSound("enemyDie");

	//add to our score count
	Glob::globsKilled++;
}

void Glob::findNearestTarget()
{
	target = nullptr;
	
	//find closest target
	for (auto entity = Entity::entities.begin(); entity != Entity::entities.end(); entity++)
	{
		if ((*entity)->type == "hero" && (*entity)->active)
		{
			//if found first hero in list, just set them as the target for now
			if (target == nullptr)
			{
				target = (LivingEntity*)(*entity); //if can't cast to living entity, it'll throw casting exception
			}
			else
			{
				//otherwise, is this other hero closer than the previous target 
				if (Entity::distanceBetweenTwoEntities(this, (*entity)) < Entity::distanceBetweenTwoEntities(this, target))
				{
					target = (LivingEntity*)(*entity);
				}
			}
		}
	}
}
void Glob::changeAnimation(int newState, bool resetFrameToBeginning)
{
	state = newState;

	if (state == GLOB_STATE_IDLE)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(GLOB_ANIM_IDLE_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(GLOB_ANIM_IDLE_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_IDLE_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_IDLE_RIGHT);
	}
	else if (state == GLOB_STATE_MOVE)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(GLOB_ANIM_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(GLOB_ANIM_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_RIGHT);
	}
	else if (state == GLOB_STATE_ATTACK)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(GLOB_ANIM_ATTACK_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(GLOB_ANIM_ATTACK_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_ATTACK_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_ATTACK_RIGHT);
	}
	else if (state == GLOB_STATE_TELEGRAPH)
	{
		if (direction == DIR_DOWN)
			currentAnim = animSet->getAnimation(GLOB_ANIM_TELEGRAPH_DOWN);
		else if (direction == DIR_UP)
			currentAnim = animSet->getAnimation(GLOB_ANIM_TELEGRAPH_UP);
		else if (direction == DIR_LEFT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_TELEGRAPH_LEFT);
		else if (direction == DIR_RIGHT)
			currentAnim = animSet->getAnimation(GLOB_ANIM_TELEGRAPH_RIGHT);
	}
	else if (state == GLOB_STATE_DEAD)
	{
		currentAnim = animSet->getAnimation(GLOB_ANIM_DIE);
	}

	if (resetFrameToBeginning)
		currentFrame = currentAnim->getFrame(0);
	else
		currentFrame = currentAnim->getFrame(currentFrame->frameIndex);
}

void Glob::updateAnimation()
{
	if (currentFrame == nullptr || currentAnim == NULL)
		return;

	//if we're in moveState but not actually going anywhere then
	if (state == GLOB_STATE_MOVE && !moving)
		changeAnimation(GLOB_STATE_IDLE, true);
	
	//if we're idle but we're actually moving, then
	if (state != GLOB_STATE_MOVE && moving)
	{
		changeAnimation(GLOB_STATE_MOVE, true);
	}

	frameTimer += TimeController::timeController.dT;

	//time to change frames
	if (frameTimer >= currentFrame->duration)
	{
		//if at the end of the animation
		if (currentFrame->frameIndex == currentAnim->getEndFrameIndex())
		{
			if (state == GLOB_STATE_TELEGRAPH)
			{
				//done telegraphing, now attack
				attack();
			}
			else if (state == GLOB_STATE_ATTACK) //if they've finished their attack
				changeAnimation(GLOB_STATE_MOVE, true);
			else if (state == GLOB_STATE_DEAD)
			{
				frameTimer = 0;
				//if somehow alive again, then cgange state back to moving
				if (hp > 0)
					changeAnimation(GLOB_STATE_MOVE, true);
				else
					active = false;
			}
			else
			{
				//loop animation
				currentFrame = currentAnim->getFrame(0);
			}
		}
		else
		{
			//otherwise just move to the next frame
			currentFrame = currentAnim->getNextFrame(currentFrame);
		}
		frameTimer = 0; //reset the frame timer for the next frame
	}
}



void Glob::updateDamages()
{
	if (active && hp > 0 && invincibleTimer <= 0)
	{
		for (auto entity = Entity::entities.begin(); entity != Entity::entities.end(); entity++)
		{
			if ((*entity)->active && (*entity)->type == "hero")
			{
				//reference as LivingEntity, so we can access hitBox/damage
				LivingEntity* enemy = (LivingEntity*)(*entity);

				//if enemy hits me, then
				if (enemy->damage > 0 && Entity::checkCollision(collisionBox, enemy->hitBox))
				{
					hp -= enemy->damage;

					if (hp > 0)
					{
						SoundManager::soundManager.playSound("enemyHit");
						invincibleTimer = 0.1;
					}
					//angle from other entity, towards this entity (get thrown backwards)
					slideAngle = Entity::angleBetweenTwoEntities((*entity), this);
					slideAmount = 300;
				}
			}
		}
	}
}