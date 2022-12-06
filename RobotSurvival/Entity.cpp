#include "entity.h"

//reference constants
const int Entity::DIR_UP=0, Entity::DIR_DOWN=1, Entity::DIR_LEFT=2, Entity::DIR_RIGHT=3, Entity::DIR_NONE=-1;

void Entity::update() {}

void Entity::draw() //override me if you want something else or more specific to happen
{
	//draws current frame
	if (currentFrame != NULL && active)
		currentFrame->Draw(animSet->spriteSheet, x, y);

	//draw collisionBox
	if (solid && Globals::debugging)
	{
		//sets the current drawing color (doesn't affect textures and whatnot)
		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(Globals::renderer, &collisionBox);
	}
}

void Entity::move(float p_angle)
{
	moving = true;
	moveSpeed = moveSpeedMax;
	angle = p_angle;

	int newDirection = angleToDirection(p_angle);

	//if direction changed, update current animation
	if (direction != newDirection)
	{
		direction = newDirection;
		changeAnimation(state, false);
	}
}

void Entity::updateMovement()
{
	updateCollisionBox();

	//store collisionBox before we move
	lastCollisionBox = collisionBox;

	//reset total moves
	totalXMove = 0;
	totalYMove = 0;

	if (moveSpeed > 0)
	{
		//works our move distance using speed, dt(time since last loop) and multiplies by lerp
		//if moveSpeed is like, 60, multiply by dT, which is usually 1ms or so
		float moveDist = moveSpeed * (TimeController::timeController.dT) * moveLerp;
		if (moveDist > 0)
		{
			//xmove = distance * cos (angle in radians)
			float xMove = moveDist * (cos(angle * Globals::PI / 180));
			//ymove = distance * sin (angle in radians)
			float yMove = moveDist * (sin(angle * Globals::PI / 180));

			x += xMove;
			y += yMove;

			totalXMove = xMove;
			totalYMove = yMove;

			//allows us to slide to a halt
			if (!moving)
				moveSpeed -= moveDist;
		}
	}
	//sliding around
	if (slideAmount > 0)
	{
		float slideDist = slideAmount * TimeController::timeController.dT * moveLerp;
		if (slideDist > 0)
		{
			float xMove = slideDist * (cos(slideAngle * Globals::PI / 180));
			float yMove = slideDist * (sin(slideAngle * Globals::PI / 180));

			x += xMove;
			y += yMove;

			totalXMove += xMove;
			totalYMove += yMove;
			slideAmount -= slideDist;
		}
		else
		{
			slideAmount = 0;
		}
	}
	//now that we've moved, move the collisionBox up to where we are now
	updateCollisionBox();
	//to help with collision checking, union collisionBox with lastCollisionBox
	SDL_UnionRect(&collisionBox, &lastCollisionBox, &collisionBox);
}

void Entity::updateCollisionBox()
{
	collisionBox.x = x - collisionBox.w / 2;
	collisionBox.y = y + collisionBoxYOffset;
	collisionBox.w = collisionBoxW;
	collisionBox.h = collisionBoxH;
}

void Entity::updateCollisions() //how we bump into stuff in the world
{
	//collisions are NOT checked if the entity isn't moving
	//change this if we have enemies that don't move, projectiles that attack the player, etc.
	if (active && collideWithSolids && (moveSpeed > 0 || slideAmount > 0))
	{
		//list of potential collisions
		list<Entity*> collisions;

		//list<Entity*>::iterator entity = ...
		for (auto entity = Entity::entities.begin(); entity != Entity::entities.end(); entity++)
		{
			//if we collide with this entity with our currently unioned collisionBox, add to the list
			//enemies of the same type, no need to check collision, walls don't check other walls,
			//players can pass through other players, etc.
			if ((*entity)->active
				&& (*entity)->type != this->type
				&& (*entity)->solid
				&& Entity::checkCollision(collisionBox, (*entity)->collisionBox))
			{
				//add it to the list
				collisions.push_back(*entity);
			}
		}
		//if we have a list of potential entities we may hit, let's check them properly to do collision resolution
		if (collisions.size() > 0)
		{
			//don't want collision box to be the unioned box anymore
			updateCollisionBox(); //set up so that we update to the new collision box (the entity's box after it has moved)

			//multisample check for collisions from where we started to where we are planning to go to

			//first we are going to find the sample distance we should travel between checks
			float boxTravelSize = 0;
			if (collisionBox.w < collisionBox.h)
				boxTravelSize = collisionBox.w / 4;
			else
				boxTravelSize = collisionBox.h / 4;

			//using this, we create a new collision box along the path at which the entity travelled
			//the new collision box is offset by boxTravelSize along the path of movement
			
			//use sampleBox to check for collisions from start point to end point, moving at boxTravelSize each sample
			SDL_Rect sampleBox = lastCollisionBox;
			float collisionMoveX = sampleBox.x, collisionMoveY = sampleBox.y;
			
			//the angle of the path of movement, ie 0 if moved right, 90 if moved up, etc.
			float movementAngle = Entity::angleBetweenTwoRects(lastCollisionBox, collisionBox);

			bool foundCollision = false;

			while (!foundCollision)
			{
				//check sampleBox for collisions where it is now
				SDL_Rect intersection;
				for (auto entity = collisions.begin(); entity != collisions.end(); entity++)
				{
					//true if intersection, false if not
					if (SDL_IntersectRect(&sampleBox, &(*entity)->collisionBox, &intersection))
					{
						foundCollision = true;
						moveSpeed = 0; //can't keep moving through a solid thing
						moving = false;

						//ex, the angle between the hero and an enemy he/she can push and make slide
						slideAngle = angleBetweenTwoEntities((*entity), this);

						//currently intersecting an entity, now we need to do collision resolution
						if (intersection.w < intersection.h)
						{
							if (lastCollisionBox.x + lastCollisionBox.w / 2 < (*entity)->collisionBox.x + (*entity)->collisionBox.w / 2)
								sampleBox.x -= intersection.w; //started on left, so move left out of collision
							else
								sampleBox.x += intersection.w; //otherwise, started on right
						}
						else
						{
							if (lastCollisionBox.y + lastCollisionBox.h / 2 < (*entity)->collisionBox.y + (*entity)->collisionBox.h / 2)
								sampleBox.y -= intersection.h; //started above, so move up out of collision
							else
								sampleBox.y += intersection.h; //otherwise, started below
						}
					}
				}

				//if collisionsfound or sampleBox is at same place as collision box, exit loop
				if (foundCollision || (sampleBox.x == collisionBox.x && sampleBox.y == collisionBox.y))
					break;

				//move sampleBox up to check the next spot
				if (distanceBetweenTwoRects(sampleBox, collisionBox) > boxTravelSize)
				{
					movementAngle = Entity::angleBetweenTwoRects(sampleBox, collisionBox);
					float xMove = boxTravelSize * (cos(movementAngle * Globals::PI / 180));
					float yMove = boxTravelSize * (sin(movementAngle * Globals::PI / 180));

					//fix for float values below 1 e.g. 0.7 being truncated to sampleBox.x as that's an int
					collisionMoveX += xMove;
					collisionMoveY += yMove;
					sampleBox.x = collisionMoveX;
					sampleBox.y = collisionMoveY;
				}
				else
				{
					sampleBox = collisionBox;
				}
			}

			if (foundCollision)
			{
				//move our entity to where the sampleBox ended up
				slideAmount = slideAmount / 2; //makes you slide slower after the initial push back
				x = sampleBox.x + sampleBox.w / 2;
				y = sampleBox.y - collisionBoxYOffset;
			}

			updateCollisionBox();
		}
	}
}

//HELPER FUNCTIONS
float Entity::distanceBetweenTwoRects(SDL_Rect& r1, SDL_Rect& r2)
{
	SDL_Point e1, e2;
	e1.x = r1.x + r1.w / 2;
	e1.y = r1.y + r1.h / 2;

	e2.x = r2.x + r2.w / 2;
	e2.y = r2.y + r2.h / 2;

	float dist = abs(sqrt(pow(e2.x - e1.x, 2) + pow(e2.y - e1.y, 2)));
	return dist;
}

float Entity::distanceBetweenTwoEntities(Entity* e1, Entity* e2)
{
	float dist = abs(sqrt(pow(e2->x - e1->x, 2) + pow(e2->y - e1->y, 2)));
	return dist;
}

float Entity::angleBetweenTwoEntities(Entity* e1, Entity* e2)
{
	float dx, dy;
	float x1 = e1->x, y1 = e1->y, x2 = e2->x, y2 = e2->y;

	dx = x2 - x1;
	dy = y2 - y1;

	return atan2(dy, dx) * 180 / Globals::PI;
}

bool Entity::checkCollision(SDL_Rect cbox1, SDL_Rect cbox2)
{
	SDL_Rect intersection;
	if (SDL_IntersectRect(&cbox1, &cbox2, &intersection))
		return true;
	
	//if a rectangle is in another rectangle
		//do it here

	//15:20 he says he'll give the code

	return false;
}

int Entity::angleToDirection(float angle)
{
	if ((angle >= 0 && angle <= 45) || (angle >= 315 && angle <= 360))
		return DIR_RIGHT;
	else if (angle >= 45 && angle <= 135)
		return DIR_DOWN;                //CONFUSED - WHY IS THIS DOWN???
	else if (angle >= 135 && angle <= 225)
		return DIR_LEFT;
	else
		return DIR_UP;
}

float Entity::angleBetweenTwoPoints(float cx1, float cy1, float cx2, float cy2)
{
	float dx = cx2 - cx1;
	float dy = cy2 - cy1;

	return atan2(dy, dx) * 180 / Globals::PI;
}

float Entity::angleBetweenTwoRects(SDL_Rect& r1, SDL_Rect& r2)
{
	float cx1 = r1.x + (r1.w / 2);
	float cy1 = r1.y + (r1.h / 2);
	float cx2 = r2.x + (r2.w / 2);
	float cy2 = r2.y + (r2.h / 2);

	return angleBetweenTwoPoints(cx1, cy1, cx2, cy2);
}

//global entities list I can refer to at any time
list<Entity*> Entity::entities;

//USED TO SORT ENTITIES BY Y-VALUE TO ORDER THEM FOR WRITING TO THE SCREEN
bool Entity::EntityCompare(const Entity* const& a, const Entity* const& b) //compare 2 entities in a list to help sorting (sorts based on y value)
{
	if (a != 0 && b != 0)
		return (a->y < b->y);
	else
		return false;
}

void Entity::removeInactiveEntitiesFromList(list<Entity*>* entityList, bool deleteEntities)
{
	for (auto entity = entityList->begin(); entity != entityList->end();)
	{
		//if entity is not active
		if (!(*entity)->active)
		{
			if (deleteEntities)
				delete(*entity);

			//setting the iterator to the next item after deletion
			entity = entityList->erase(entity);
		}
		else
			entity++;
	}
}

void Entity::removeAllFromList(list<Entity*>* entityList, bool deleteEntities)
{
	for (auto entity = entityList->begin(); entity != entityList->end();)
	{
		if (deleteEntities)
			delete(*entity);

		//setting the iterator to the next item after deletion
		entity = entityList->erase(entity);
	}
}