#include "livingEntity.h"

void LivingEntity::updateHitBox()
{
	//assume damage is 0 for now
	damage = 0;


	//int x = (int)4.5; We are casting, we are turning a GroupBox into a GroupBox pointer I think?
	GroupBox* hitBoxes = (GroupBox*)GroupBuilder::findGroupByName("hitBox", currentFrame->frameData);
	if (hitBoxes != NULL && hitBoxes->numberOfDataInGroup() > 0)
	{
		//update hitbox
		SDL_Rect hb = hitBoxes->data.front(); //gives us the first SDL_Rect in our hitBoxes group
		                                      //this game only deals with 1 hitbox
		hitBox.x = x - currentFrame->offSet.x + hb.x;
		hitBox.y = y - currentFrame->offSet.y + hb.y;
		hitBox.w = hb.w;
		hitBox.h = hb.h;

		//update damage
		//will find a group and turn it into a groupnumber pointer
		GroupNumber* damages = (GroupNumber*)GroupBuilder::findGroupByName("damage", currentFrame->frameData);
		if (damages != NULL && damages->numberOfDataInGroup() > 0)
			damage = damages->data.front();
	}
}

void LivingEntity::updateInvincibleTimer()
{
	if (invincibleTimer > 0)
		invincibleTimer -= TimeController::timeController.dT;
}

void LivingEntity::draw() //overriding entity's draw
{
	//draws current frame
	if (currentFrame != NULL && active)
	{
		if (invincibleTimer > 0 && animSet->whiteSpriteSheet != nullptr)
			currentFrame->Draw(animSet->whiteSpriteSheet, x, y);
		else
			currentFrame->Draw(animSet->spriteSheet, x, y);
	}

	//draw collisionBox
	if (solid && Globals::debugging)
	{
		//sets the current drawing color (doesn't affect textures and whatnot)
		SDL_SetRenderDrawColor(Globals::renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(Globals::renderer, &lastCollisionBox);

		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(Globals::renderer, &collisionBox);

		SDL_SetRenderDrawColor(Globals::renderer, 255, 0, 255, SDL_ALPHA_OPAQUE);
		SDL_RenderDrawRect(Globals::renderer, &hitBox);
	}
}