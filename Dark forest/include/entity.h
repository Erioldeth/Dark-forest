#pragma once
#ifndef ENTITY
#define ENTITY
#include"const_value.h"
#include"texture.h"
#include"randomMachine.h"
enum entityMovement {
	STAY_STILL,
	MOVE_LEFT,
	MOVE_RIGHT,
	JUMP,
	SIT,
	TOTAL_MOVEMENT
};
class Entity {
	int ID, posX, posY, velX, velY, entityVel, currentSprite, currenSubSprite;
	bool autonomous, followPlayer, onGround, sit, motionDelay;

	Texture entityTexture;
	SDL_Rect collideRegion;
	SDL_RendererFlip flip;
	std::vector<std::vector<SDL_Rect>> entitySpriteClip;
	std::vector<int> subSprite;

	Mix_Chunk* moveAround;
	Mix_Chunk* die;
public:
	Entity() {
		ID = 0;
		posX = posY = 0;
		velX = velY = entityVel = 0;
		collideRegion = {0,0,0,0};

		currenSubSprite = 0;
		currentSprite = STAY_STILL;

		moveAround = die = nullptr;

		motionDelay = autonomous = followPlayer = onGround = sit = false;
		flip = SDL_FLIP_NONE;

		subSprite.resize(TOTAL_MOVEMENT);
		entitySpriteClip.resize(TOTAL_MOVEMENT);
	}
	void setAuto() {
		autonomous = true;
	}
	void setFollowPlayer() {
		followPlayer = true;
	}
	void setID(int id) {
		ID = id;
	}
	void setVeclocity(int v = 0) {
		if(autonomous) {
			if(followPlayer) {
				std::uniform_int_distribution<> Rand(1, 2);
				entityVel = Rand(mt);
			}
			else {
				std::uniform_int_distribution<> Rand(3, 5);
				entityVel = Rand(mt);
			}
		}
		else entityVel = v;
	}
	void loadEntityTexture(std::string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		entityTexture.loadFromFile(path, r, g, b);
	}
	void setAmountOfSprite(int mainSprite, int n) {
		subSprite[mainSprite] = n;
		entitySpriteClip[mainSprite].resize(n);
	}
	void setEntitySprite(int mainSprite, int subSprite, int x, int y, int w, int h) {
		entitySpriteClip[mainSprite][subSprite] = {x,y,w,h};
	}
	void setEntitySize(int w, int h) {
		entityTexture.setSize(w, h);
		collideRegion.w = w;
		collideRegion.h = h;
	}
	void setPosition(int x = 0, int y = 0) {
		if(autonomous) {
			std::uniform_int_distribution<> horizontal(0, 1);
			bool left = horizontal(mt);
			if(left) {
				collideRegion.x = posX = -entityTexture.getWidth();
				flip = SDL_FLIP_NONE;
			}
			else {
				collideRegion.x = posX = SCREEN_WIDTH;
				flip = SDL_FLIP_HORIZONTAL;
			}
			std::uniform_int_distribution<> vertical(50, 570 - entityTexture.getHeight());
			collideRegion.y = posY = vertical(mt);
			if(followPlayer) collideRegion = {posX + 10,posY + 5,28,40};
		}
		else {
			collideRegion.x = posX = x;
			collideRegion.y = posY = y;
		}
	}
	void loadMoveSound(std::string path, Uint8 volume) {
		Mix_FreeChunk(moveAround);
		moveAround = nullptr;
		moveAround = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(moveAround, volume);
	}
	void loadDeadSound(std::string path, Uint8 volume) {
		Mix_FreeChunk(die);
		die = nullptr;
		die = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(die, volume);
	}
	void renderEntity() {
		entityTexture.render(posX, posY, &entitySpriteClip[currentSprite][currenSubSprite], 0.0, nullptr, flip);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_RenderDrawRect(renderer, &collideRegion);
	}
	int getSprite() { return currentSprite; };
	int getNumberSubSprite(int mainSprite) { return subSprite[mainSprite]; };
	int entityWidth() { return entityTexture.getWidth(); };
	int entityHeight() { return entityTexture.getHeight(); };
	void free() {
		entityTexture.free();

		for(auto& i : entitySpriteClip) i.resize(0);
		entitySpriteClip.resize(0);
		subSprite.resize(0);

		Mix_FreeChunk(moveAround);
		Mix_FreeChunk(die);
		moveAround = die = nullptr;
	}
	void handleEvent(SDL_Event* e) {
		int oldSprite = currentSprite;
		if(e->type == SDL_KEYDOWN && !e->key.repeat) {
			switch(e->key.keysym.sym) {
				case SDLK_a:
					velX -= entityVel;
					break;
				case SDLK_d:
					velX += entityVel;
					break;
				case SDLK_w:
					if(onGround && !sit) {
						velY = -15;
						onGround = false;
					}
					break;
				case SDLK_s:
					if(onGround) {
						sit = true;
						posY += entityHeight() * 1 / 3;
						entityTexture.setSize(entityWidth(), entityHeight() * 2 / 3);
						collideRegion.y = posY;
						collideRegion.h = entityHeight();
					}
					break;
			}
		}
		else if(e->type == SDL_KEYUP && !e->key.repeat) {
			switch(e->key.keysym.sym) {
				case SDLK_a:
					velX += entityVel;
					break;
				case SDLK_d:
					velX -= entityVel;
					break;
				case SDLK_s:
					if(sit) {
						sit = false;
						entityTexture.setSize(entityWidth(), entityHeight() * 3 >> 1);
						posY -= entityHeight() * 1 / 3;
						collideRegion.y = posY;
						collideRegion.h = entityHeight();
					}
					break;
			}
		}
		//change animation according to action
		if(velX == 0) currentSprite = STAY_STILL;
		else if(velX > 0) {
			currentSprite = MOVE_RIGHT;
			flip = SDL_FLIP_NONE;
		}
		else {
			currentSprite = MOVE_LEFT;
			flip = SDL_FLIP_HORIZONTAL;
		}
		if(!onGround) currentSprite = JUMP;
		if(sit) currentSprite = SIT;
		//reset animation frame
		if(currentSprite != oldSprite) currenSubSprite = 0;
	}
	bool collided(Entity& player) {
		int leftA = collideRegion.x,
			rightA = leftA + collideRegion.w,
			topA = collideRegion.y,
			bottomA = topA + collideRegion.h;

		int leftB = player.collideRegion.x,
			rightB = leftB + player.collideRegion.w,
			topB = player.collideRegion.y,
			bottomB = topB + player.collideRegion.h;

		if(leftA >= rightB || rightA <= leftB || bottomA <= topB || topA >= bottomB) return false;

		if(followPlayer) {
			if(leftB < rightA && rightB - (player.entityWidth() >> 2) >= leftA && leftB + (player.entityWidth() >> 2) <= rightA && bottomB > topA && topB < topA && !player.onGround) {
				Mix_HaltChannel(3);
				Mix_PlayChannel(3, die, 0);
				player.velY = -10;
				setVeclocity();
				setPosition();
				reaper_curse[ID] = reaper_creation();
				return false;
			}
		}
		return true;
	}
	void move(std::vector<SDL_Rect>& platform, Entity& player) {
		if(!motionDelay) {
			//for bot movement
			if(autonomous) {
				Uint16 create_sound = sound_play(mt);

				if(followPlayer) {
					if(create_sound == 128 && !Mix_Playing(3)) {
						std::uniform_int_distribution<> file(1, 3);
						Uint16 chosen_sound = file(mt);
						std::string path = "sound/reaper_move" + std::to_string(chosen_sound) + ".wav";
						loadMoveSound(path, MAX_VOLUME >> 2);
						Mix_PlayChannel(3, moveAround, 0);
					}

					if(posX < player.posX) {
						posX += entityVel;
						flip = SDL_FLIP_NONE;
					}
					else if(posX > player.posX) {
						posX -= entityVel;
						flip = SDL_FLIP_HORIZONTAL;
					}
					if(posY > player.posY) posY -= entityVel;
					else if(posY < player.posY) posY += entityVel;

					collideRegion.x = posX + 10;
					collideRegion.y = posY + 5;

					if(collided(player)) SHADOW_CAUGHT = DEAD = true;
				}
				else {
					if(create_sound == 128 && !Mix_Playing(4)) {
						std::uniform_int_distribution<> file(1, 3);
						Uint16 chosen_sound = file(mt);
						std::string path = "sound/skull_move" + std::to_string(chosen_sound) + ".wav";
						loadMoveSound(path, MAX_VOLUME >> 2);
						Mix_PlayChannel(4, moveAround, 0);
					}
					posX += (flip == SDL_FLIP_NONE ? entityVel : -entityVel);

					collideRegion.x = posX;

					if((flip == SDL_FLIP_NONE && posX > SCREEN_WIDTH) || (flip == SDL_FLIP_HORIZONTAL && posX < -entityTexture.getWidth())) {
						setVeclocity();
						setPosition();
						skull_curse[ID] = skull_creation();
					}

					if(collided(player)) EXPLODED = DEAD = true;
				}
			}
			//for player movement
			else {
				if(!sit && velX != 0) {
					posX += velX;
					if(!Mix_Playing(2) && onGround) Mix_PlayChannel(2, moveAround, 0);
				}
				//else Mix_HaltChannel(2);
				else Mix_FadeOutChannel(2, 400);
				if(!onGround) {
					posY += velY;
					velY += ACCELERATOR;
					if(velY > 15) velY = 15;
					Mix_HaltChannel(2);
				}
				//check player collide with any platform
				bool collideBottom = false;
				for(auto& collider : platform) {
					int leftA = posX,
						rightA = posX + entityTexture.getWidth(),
						topA = posY,
						bottomA = posY + entityTexture.getHeight();

					int leftB = collider.x,
						rightB = collider.x + collider.w,
						topB = collider.y,
						bottomB = collider.y + collider.h;
					//vertical collide
					if(leftA < rightB && rightA > leftB) {
						if(bottomA >= topB && topA + (entityTexture.getHeight() >> 1) < topB) {
							bottomA = topB;
							topA = posY = topB - entityTexture.getHeight();
							collideBottom = true;
						}
						else if(topA <= bottomB && bottomA - (entityTexture.getHeight() >> 1) > bottomB) {
							topA = posY = bottomB;
							bottomA = bottomB + entityTexture.getHeight();
							velY = 1;
						}
					}
					//horizontal collide
					if(bottomA > topB && topA < bottomB) {
						if(leftA <= rightB && rightA > rightB) {
							leftA = posX = rightB;
							rightA = rightB + entityTexture.getWidth();
						}
						else if(rightA >= leftB && leftA < leftB) {
							rightA = leftB;
							leftA = posX = leftB - entityTexture.getWidth();
						}
					}
				}
				if(!collideBottom && posY + entityTexture.getHeight() < SCREEN_HEIGHT) onGround = false;
				else onGround = true;
				//check player in screen
				if(posX < 0) {
					posX = 0;
				}
				else if(posX + entityTexture.getWidth() > SCREEN_WIDTH) {
					posX = SCREEN_WIDTH - entityTexture.getWidth();
				}
				if(posY < 0) {
					posY = 0;
					velY = 1;
				}
				else if(posY + entityTexture.getHeight() > SCREEN_HEIGHT) {
					posY = SCREEN_HEIGHT - entityTexture.getHeight();
					onGround = true;
				}
				//check state (on ground/on air) to change current state
				if(onGround) {
					velY = 0;
					if(!sit) {
						if(velX == 0) currentSprite = STAY_STILL;
						else if(velX > 0) currentSprite = MOVE_RIGHT;
						else currentSprite = MOVE_LEFT;
					}
				}
				collideRegion.x = posX;
				collideRegion.y = posY;
			}
		}
		currenSubSprite = (currenSubSprite + 1) % subSprite[currentSprite];
		motionDelay = (motionDelay ? false : true);
	}
	void alert() {
		if(followPlayer) reaper_warning.render((posX < 0 ? 0 : SCREEN_WIDTH - reaper_warning.getWidth()), collideRegion.y - ((reaper_warning.getWidth() - collideRegion.h) >> 1));
		else skull_warning.render((posX < 0 ? 0 : SCREEN_WIDTH - skull_warning.getWidth()), collideRegion.y - ((skull_warning.getWidth() - collideRegion.h) >> 1));
	}
	SDL_Point entityPosition() {
		return {posX,posY};
	}
	void resetEntity() {
		currentSprite = STAY_STILL;
		currenSubSprite = 0;
		if(!autonomous) velX = velY = 0;
	}
};

//game play
Entity player;
std::vector<Entity> skull(MAX_SKULL), reaper(MAX_REAPER);

#endif // !ENTITY
