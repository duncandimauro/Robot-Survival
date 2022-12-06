#ifndef ENTITY_H
#define ENTITY_H

#include "globals.h"
#include "timeController.h"
#include "animationSet.h"

//Abstract Class - cannot instansiate an object of type Entity e.g. cannot do Entity e;
class Entity
{
public:
	//reference constants
	static const int DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE;

	//quick label to see what the entity is up to
	int state;

	float x, y;
	int direction;
	bool solid = true; //is this thing solid, can things pass through me?
	bool collideWithSolids = true; //sometimes we are solid, but I pass through other solids
	bool active = true; //entity turned on or off, used to delete, make things go away then come back, etc.
	string type = "entity"; //what type of entity is it? e.g. hero, enemy, wall, etc.
	bool moving; //is the entity moving
	float angle; //angle to move the entity in (360 degree angle)
	float moveSpeed;
	float moveSpeedMax;
	float slideAngle; //they've been pushed by something, direciton pushed in
	float slideAmount; //amount of push in the slideAngle
	float moveLerp = 4;
	float totalXMove, totalYMove; //keeps track of total x,y movement per movement turn. Just in case we need to retract movement

	SDL_Rect collisionBox; //box describing the size of our entity and this is used to bump into things
	SDL_Rect lastCollisionBox; //where out collisionbox was last
	int collisionBoxW, collisionBoxH; //our default collisionBox sizes
	float collisionBoxYOffset; //from my entities y value, where should I sit this collisionBox

	AnimationSet* animSet; //set of all animations this entity can have
	Animation* currentAnim; //current animation the entity is using
	Frame* currentFrame; //the current frame in the above animation the entity using
	float frameTimer; //help animate frame to frame

	//VIRTUAL FUNCTIONS
	virtual void update();
	virtual void draw();

	virtual void move(float angle);
	virtual void updateMovement();
	virtual void updateCollisionBox();

	//let's say you're the hero, you're running right, then you run up, set the newState to run up and 
	//keep the frame that you're on, so, the 3rd running frame right will change to the 3rd running frame up
	virtual void changeAnimation(int newState, bool resetFrameToBeginning) = 0; //abstract function
	virtual void updateCollisions(); //how we bump into stuff in the world

	//HELPER FUNCTIONS
	static float distanceBetweenTwoRects(SDL_Rect& r1, SDL_Rect& r2);
	static float distanceBetweenTwoEntities(Entity* e1, Entity* e2);
	static float angleBetweenTwoEntities(Entity* e1, Entity* e2);
	static bool checkCollision(SDL_Rect cbox1, SDL_Rect cbox2);
	static int angleToDirection(float angle);
	static float angleBetweenTwoPoints(float cx1, float cy1, float cx2, float cy2);
	static float angleBetweenTwoRects(SDL_Rect& r1, SDL_Rect& r2);

	//global entities list I can refer to at any time
	static list<Entity*> entities;
	static bool EntityCompare(const Entity* const& a, const Entity* const& b); //compare 2 entities in a list to help sorting (sorts based on y value)
	static void removeInactiveEntitiesFromList(list<Entity*>* entityList, bool deleteEntities);
	static void removeAllFromList(list<Entity*>* entityList, bool deleteEntities);
};

#endif