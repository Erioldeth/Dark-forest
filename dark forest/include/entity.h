//#pragma once
#ifndef ENTITY
#define ENTITY
#include"const_value.h"
#include"particle.h"
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
	bool autonomous, followPlayer, onGround, jump, sit, motionDelay;

	Texture entityTexture;

	SDL_Rect collideRegion;
	SDL_RendererFlip flip;
	std::vector<std::vector<SDL_Rect>> entitySpriteClip;
	std::vector<int> subSprite;

	Particle entityParticle;

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

		motionDelay = autonomous = followPlayer = onGround = jump = sit = false;
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
		if(autonomous) {
			if(followPlayer) entityParticle.loadParticleTexture("texture/reaper_particle.png", 0, 255, 0);
			else entityParticle.loadParticleTexture("texture/skull_particle.png", 0, 255, 0);
			entityParticle.setParticleSize(20, 20);
		}
		else {
			entityParticle.loadParticleTexture("texture/player_particle.png");
			entityParticle.setParticleSize(15, 15);
		}
	}
	void setNumberAnimation(int mainSprite, int n) {
		subSprite[mainSprite] = n;
		entitySpriteClip[mainSprite].resize(n);
	}
	void setEntityAnimation(int mainSprite, int subSprite, int x, int y, int w, int h) {
		entitySpriteClip[mainSprite][subSprite] = {x,y,w,h};
	}
	void setEntitySize(int w, int h) {
		entityTexture.setSize(w, h);
	}
	void setPosition(int x = 0, int y = 0) {
		entityParticle.reset();
		if(autonomous) {
			std::uniform_int_distribution<> horizontal(0, 1);
			bool left = horizontal(mt);
			if(left) {
				collideRegion.x = posX = -entityTexture.textureWidth() - 25;
				flip = SDL_FLIP_NONE;
			}
			else {
				collideRegion.x = posX = SCREEN_WIDTH + 25;
				flip = SDL_FLIP_HORIZONTAL;
			}
			std::uniform_int_distribution<> vertical(50, 570 - entityTexture.textureHeight());
			collideRegion.y = posY = vertical(mt);
			if(followPlayer) collideRegion = {posX + 14,posY + 5,20,38};
			else collideRegion = {posX + 5,posY + 3,20,30};
		}
		else {
			posX = x;
			posY = y;
			collideRegion = {posX + 6,posY + 1,20,40};
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
		if(autonomous) {
			SDL_Rect region = {posX,posY,entityTexture.textureWidth(),entityTexture.textureHeight()};
			if(followPlayer) entityParticle.render(region, 24, 8, 256, 256);
			else entityParticle.render(region, 16, 4, 128, 128);
		}
		else {
			SDL_Rect region = {posX + 6,posY + 11,20,20};
			entityParticle.render(region, 40, 8, 70, 70);
		}
		entityTexture.render(posX, posY, &entitySpriteClip[currentSprite][currenSubSprite], 0.0, nullptr, flip);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_RenderDrawRect(renderer, &collideRegion);
	}
	int getCurrentSprite() { return currentSprite; };
	int getNumberSubSprite(int mainSprite) { return subSprite[mainSprite]; };
	int entityWidth() { return entityTexture.textureWidth(); };
	int entityHeight() { return entityTexture.textureHeight(); };
	void free() {
		entityTexture.free();

		for(auto& sc : entitySpriteClip) sc.clear();
		entitySpriteClip.clear();
		subSprite.clear();

		entityParticle.free();

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
					if(onGround && !sit && !jump) {
						velY = -15;
						onGround = false;
						jump = true;
					}
					break;
				case SDLK_s:
					if(onGround && !jump) {
						sit = true;
						posY += entityHeight() * 1 / 3;
						entityTexture.setSize(entityWidth(), (entityHeight() << 1) / 3);
						collideRegion.y = posY + 1;
						collideRegion.h = entityHeight() - 2;
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
				case SDLK_w:
					jump = false;
					break;
				case SDLK_s:
					if(sit) {
						sit = false;
						entityTexture.setSize(entityWidth(), (entityHeight() * 3) >> 1);
						posY -= entityHeight() * 1 / 3;
						collideRegion.y = posY + 1;
						collideRegion.h = entityHeight() - 2;
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
	void move(std::vector<SDL_Rect>& platform) {
		if(!motionDelay) {
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
			int leftA = posX,
				rightA = posX + entityWidth(),
				topA = posY,
				bottomA = posY + entityHeight();
			for(auto& collider : platform) {
				int leftB = collider.x,
					rightB = collider.x + collider.w,
					topB = collider.y,
					bottomB = collider.y + collider.h;
				//vertical collide
				if(leftA < rightB && rightA > leftB) {
					if(bottomA >= topB && topA + (entityTexture.textureHeight() >> 1) < topB) {
						bottomA = topB;
						topA = posY = topB - entityTexture.textureHeight();
						collideBottom = true;
					}
					else if(topA <= bottomB && bottomA - (entityTexture.textureHeight() >> 1) > bottomB) {
						topA = posY = bottomB;
						bottomA = bottomB + entityTexture.textureHeight();
						velY = 1;
					}
				}
				//horizontal collide
				if(bottomA > topB && topA < bottomB) {
					if(leftA <= rightB && rightA > rightB) {
						leftA = posX = rightB;
						rightA = rightB + entityTexture.textureWidth();
					}
					else if(rightA >= leftB && leftA < leftB) {
						rightA = leftB;
						leftA = posX = leftB - entityTexture.textureWidth();
					}
				}
			}
			if(!collideBottom && posY + entityHeight() < SCREEN_HEIGHT) onGround = false;
			else onGround = true;
			//check player in screen
			if(posX < 0) {
				posX = 0;
			}
			else if(posX + entityTexture.textureWidth() > SCREEN_WIDTH) {
				posX = SCREEN_WIDTH - entityTexture.textureWidth();
			}
			if(posY < 0) {
				posY = 0;
				velY = 1;
			}
			else if(posY + entityTexture.textureHeight() > SCREEN_HEIGHT) {
				posY = SCREEN_HEIGHT - entityTexture.textureHeight();
				onGround = true;
			}
			//check state (on ground/on air) to change gravity
			if(onGround) {
				if(jump) {
					velY = -15;
					onGround = false;
				}
				else velY = 0;
				if(!sit && !jump) {
					if(velX == 0) currentSprite = STAY_STILL;
					else if(velX > 0) currentSprite = MOVE_RIGHT;
					else currentSprite = MOVE_LEFT;
				}
			}
			collideRegion.x = posX + 6;
			collideRegion.y = posY + 1;
		}
		currenSubSprite = (currenSubSprite + 1) % subSprite[currentSprite];
		motionDelay = (motionDelay ? false : true);
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
			if(rightB - (player.collideRegion.w >> 2) >= leftA && leftB + (player.collideRegion.w >> 2) <= rightA && bottomB > topA && topB < topA && !player.onGround) {
				Mix_HaltChannel(3);
				Mix_PlayChannel(3, die, 0);
				player.velY = -10;

				Texture reaper_death_texture;
				reaper_death_texture.loadFromFile("texture/reaper_death.png", 255);
				reaper_death_texture.setSize(50, 50);
				SDL_Point render_pos = {posX - ((reaper_death_texture.textureWidth() - entityWidth()) >> 1), posY - ((reaper_death_texture.textureHeight() - entityHeight()) >> 1)};
				reaper_death.push_back(std::make_tuple(reaper_death_texture, render_pos, 0));

				setVeclocity();
				setPosition();
				reaper_curse[ID] = reaper_creation();
				return false;
			}
		}
		return true;
	}
	void move(Entity& player) {
		if(!motionDelay) {
			Uint16 create_sound = sound_play(mt);
			if(followPlayer) {
				if(create_sound == 128 && !Mix_Playing(3)) {
					std::uniform_int_distribution<> file(1, 3);
					Uint16 chosen_sound = file(mt);
					std::string path = "sound/reaper_move" + std::to_string(chosen_sound) + ".wav";
					loadMoveSound(path, MAX_VOLUME >> 2);
					Mix_PlayChannel(3, moveAround, 0);
				}

				SDL_Point center = {posX + (entityWidth() >> 1),posY + (entityHeight() >> 1)};
				SDL_Point player_center = {player.posX + (player.entityWidth() >> 1),player.posY + (player.entityHeight() >> 1)};

				if(player_center.x > center.x) {
					flip = SDL_FLIP_NONE;
					posX += entityVel;
					collideRegion.x += entityVel;
				}
				else if(player_center.x < center.x) {
					flip = SDL_FLIP_HORIZONTAL;
					posX -= entityVel;
					collideRegion.x -= entityVel;
				}
				if(player_center.y > center.y) {
					posY += entityVel;
					collideRegion.y += entityVel;
				}
				else if(player_center.y < center.y) {
					posY -= entityVel;
					collideRegion.y -= entityVel;
				}

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
				collideRegion.x += (flip == SDL_FLIP_NONE ? entityVel : -entityVel);

				if(posX > SCREEN_WIDTH + 25 || posX < -entityTexture.textureWidth() - 25) {
					setVeclocity();
					setPosition();
					skull_curse[ID] = skull_creation();
				}

				if(collided(player)) EXPLODED = DEAD = true;
			}
		}
		currenSubSprite = (currenSubSprite + 1) % subSprite[currentSprite];
		motionDelay = (motionDelay ? false : true);
	}
	void alert() {
		if(followPlayer) reaper_warning.render((posX < 0 ? 0 : SCREEN_WIDTH - reaper_warning.textureWidth()), collideRegion.y - ((reaper_warning.textureHeight() - collideRegion.h) >> 1));
		else skull_warning.render((posX < 0 ? 0 : SCREEN_WIDTH - skull_warning.textureWidth()), collideRegion.y - ((skull_warning.textureHeight() - collideRegion.h) >> 1));
	}
	SDL_Point entityPosition() {
		return {posX,posY};
	}
	void resetEntity() {
		currentSprite = STAY_STILL;
		currenSubSprite = 0;
		if(autonomous) {
			if(followPlayer) reaper_curse[ID] = false;
			else skull_curse[ID] = false;
			setVeclocity();
			setPosition();
		}
		else {
			velX = velY = 0;
			onGround = jump = sit = false;
		}
	}
};

//game play
Entity player;
std::vector<Entity> skull(MAX_SKULL), reaper(MAX_REAPER);

#endif // !ENTITY
