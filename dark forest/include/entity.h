#ifndef ENTITY_H
#define ENTITY_H
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
	int ID, posX, posY, velX, velY, entityVel, oldAction, currentAction, currenMovement, invincible_time;
	bool autonomous, followPlayer, onGround, jump, sit, motionDelay, invincible;

	Texture entityTexture;

	SDL_Rect collideRegion;
	SDL_RendererFlip flip;
	std::vector<std::vector<SDL_Rect>> entitySpriteClip;

	Particle entityParticle;

	Mix_Chunk* moveAround;
	Mix_Chunk* die;
public:
	Entity() {
		ID = 0;
		posX = posY = 0;
		velX = velY = entityVel = 0;
		invincible_time = 0;
		collideRegion = {0,0,0,0};

		currenMovement = 0;
		oldAction = currentAction = STAY_STILL;

		moveAround = die = nullptr;

		motionDelay = autonomous = followPlayer = onGround = jump = invincible = sit = false;
		flip = SDL_FLIP_NONE;

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

	void loadEntityTexture(std::string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		entityTexture.loadTextureFromFile(path, r, g, b);
		if(autonomous) {
			if(followPlayer) {
				entityTexture.setTextureBlendMode(SDL_BLENDMODE_BLEND);
				entityTexture.setTextureAlpha(175);
				entityParticle.loadParticleTexture("texture/reaper_particle.png");
			}
			else entityParticle.loadParticleTexture("texture/skull_particle.png");
			entityParticle.setParticleSize(20, 20);
		}
		else {
			entityParticle.loadParticleTexture("texture/player_particle.png");
			entityParticle.setParticleSize(15, 15);
		}
	}
	void setNumberOfAnimation(int mainSprite, int n) {
		entitySpriteClip[mainSprite].resize(n);
	}
	void setEntityAnimation(int action, int movement, int x, int y, int w, int h) {
		entitySpriteClip[action][movement] = {x,y,w,h};
	}
	void setEntitySize(int w, int h) {
		entityTexture.setTextureSize(w, h);
	}
	void setEntityColor(Uint8 r, Uint8 g, Uint8 b) {
		entityTexture.setTextureColor(r, g, b);
	}
	void setParticleColor(Uint8 r, Uint8 g, Uint8 b) {
		entityParticle.setParticleColor(r, g, b);
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

	void loadMoveSound(std::string path, int volume) {
		Mix_FreeChunk(moveAround);
		moveAround = nullptr;
		moveAround = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(moveAround, volume);
	}
	void loadDeadSound(std::string path, int volume) {
		Mix_FreeChunk(die);
		die = nullptr;
		die = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(die, volume);
	}

	void renderEntity() {
		if(autonomous) {
			SDL_Rect region = {posX,posY,entityTexture.textureWidth(),entityTexture.textureHeight()};
			if(followPlayer) entityParticle.renderParticle(region, 24, 8, 256, 256);
			else entityParticle.renderParticle(region, 16, 4, 128, 128);
		}
		else {
			SDL_Rect region = {posX + 6,posY + 11,20,20};
			entityParticle.renderParticle(region, 40, 8, 70, 70);
		}
		entityTexture.renderTexture(posX, posY, &entitySpriteClip[currentAction][currenMovement], 0.0, nullptr, flip);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_RenderDrawRect(renderer, &collideRegion);
	}

	int getNumberOfAnimation(int mainSprite) { return (int)entitySpriteClip[mainSprite].size(); };

	int entityWidth() { return entityTexture.textureWidth(); };
	int entityHeight() { return entityTexture.textureHeight(); };

	void free() {
		entityTexture.free();

		for(auto& sc : entitySpriteClip) sc.clear();
		entitySpriteClip.clear();

		entityParticle.free();

		Mix_FreeChunk(moveAround);
		Mix_FreeChunk(die);
		moveAround = die = nullptr;
	}

	void handleEvent(SDL_Event* e) {
		if(e->type == SDL_KEYDOWN && !e->key.repeat) {
			switch(e->key.keysym.sym) {
				case SDLK_a:
					velX -= entityVel;
					break;
				case SDLK_d:
					velX += entityVel;
					break;
				case SDLK_w:
					if(!PAUSED && onGround && !sit && !jump) {
						velY = -15;
						onGround = false;
						jump = true;
					}
					break;
				case SDLK_s:
					if(!PAUSED && onGround && !jump) {
						sit = true;
						posY += entityHeight() * 1 / 3;
						entityTexture.setTextureSize(entityWidth(), (entityHeight() << 1) / 3);
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
						entityTexture.setTextureSize(entityWidth(), (entityHeight() * 3) >> 1);
						posY -= entityHeight() * 1 / 3;
						collideRegion.y = posY + 1;
						collideRegion.h = entityHeight() - 2;
					}
					break;
			}
		}
		//change animation according to action
		oldAction = currentAction;
		if(!PAUSED) {
			if(velX == 0) currentAction = STAY_STILL;
			else if(velX > 0) {
				currentAction = MOVE_RIGHT;
				flip = SDL_FLIP_NONE;
			}
			else {
				currentAction = MOVE_LEFT;
				flip = SDL_FLIP_HORIZONTAL;
			}
			if(!onGround) currentAction = JUMP;
			if(sit) currentAction = SIT;
		}
		if(currentAction != oldAction) currenMovement = 0;
		//reset animation frame
	}
	void move(std::vector<SDL_Rect>& platform) {
		if(!motionDelay) {
			if(!sit && velX != 0) {
				posX += velX;
				if(!Mix_Playing(2) && onGround) Mix_PlayChannel(2, moveAround, 0);
			}
			else Mix_FadeOutChannel(2, 400);
			if(!onGround) {
				if(invincible_time == 0) invincible = false;
				else invincible_time--;
				posY += velY;
				velY += ACCELERATOR;
				if(velY > 15) velY = 15;
				Mix_HaltChannel(2);
			}
			else {
				invincible = false;
				invincible_time = 0;
			}

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
			if(!collideBottom) onGround = false;
			else onGround = true;

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

			if(onGround) {
				if(jump) {
					velY = -15;
					onGround = false;
				}
				else velY = 0;
				if(!sit && !jump) {
					if(velX == 0) currentAction = STAY_STILL;
					else if(velX > 0) currentAction = MOVE_RIGHT;
					else currentAction = MOVE_LEFT;
				}
			}
			collideRegion.x = posX + 6;
			collideRegion.y = posY + 1;
		}

		currenMovement = (currenMovement + 1) % (int)entitySpriteClip[currentAction].size();
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

		if(followPlayer && rightB > leftA && leftB < rightA && bottomB > topA && topB < topA && player.velY > 0) {
			Mix_HaltChannel(3);
			Mix_PlayChannel(3, die, 0);
			player.invincible = true;
			player.invincible_time = 5;
			player.velY = -10;

			Texture reaper_death_texture;
			reaper_death_texture.loadTextureFromFile("texture/reaper_death.png", 255);
			reaper_death_texture.setTextureSize(50, 50);
			SDL_Point render_pos = {posX - ((reaper_death_texture.textureWidth() - entityWidth()) >> 1), posY - ((reaper_death_texture.textureHeight() - entityHeight()) >> 1)};
			reaper_death.emplace_back(reaper_death_texture, render_pos, 0);

			setVeclocity();
			setPosition();
			reaper_curse[ID] = reaper_creation();
			return false;
		}
		return true;
	}
	void move(Entity& player) {
		if(!motionDelay) {
			if(play_sound(mt) == 128 && ((followPlayer && !Mix_Playing(3)) || (!followPlayer && !Mix_Playing(4)))) {
				std::uniform_int_distribution<> file(1, 3);
				std::string path = (followPlayer ? "sound/reaper_move" : "sound/skull_move") + std::to_string(file(mt)) + ".wav";
				loadMoveSound(path, (followPlayer ? 2 : 3) * volume_ratio[1]);
				Mix_PlayChannel((followPlayer ? 3 : 4), moveAround, 0);
			}
			if(followPlayer) {
				SDL_Point center = {posX + (entityWidth() >> 1),posY + (entityHeight() >> 1)};
				SDL_Point player_center = {player.posX + (player.entityWidth() >> 1),player.posY + (player.entityHeight() >> 1)};

				flip = (player_center.x > center.x ? SDL_FLIP_NONE : SDL_FLIP_HORIZONTAL);

				std::vector<double> vec{double(player_center.x - center.x),double(player_center.y - center.y)};

				double hypotenuse = sqrt(vec[0] * vec[0] + vec[1] * vec[1]);
				int Xchange = int(round((double)entityVel * (vec[0] / hypotenuse)));
				int Ychange = int(round((double)entityVel * (vec[1] / hypotenuse)));
				posX += Xchange, collideRegion.x += Xchange;
				posY += Ychange, collideRegion.y += Ychange;

				if(collided(player) && !player.invincible) SHADOW_CAUGHT = DEAD = true;
			}
			else {
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
		currenMovement = (currenMovement + 1) % (int)entitySpriteClip[currentAction].size();
		motionDelay = (motionDelay ? false : true);
	}
	void alert() {
		if(followPlayer) reaper_warning.renderTexture((posX < 0 ? 0 : SCREEN_WIDTH - reaper_warning.textureWidth()), collideRegion.y - ((reaper_warning.textureHeight() - collideRegion.h) >> 1));
		else skull_warning.renderTexture((posX < 0 ? 0 : SCREEN_WIDTH - skull_warning.textureWidth()), collideRegion.y - ((skull_warning.textureHeight() - collideRegion.h) >> 1));
	}

	SDL_Point entityPosition() {
		return {posX,posY};
	}
	void resetEntity() {
		currenMovement = 0;
		if(autonomous) {
			if(followPlayer) reaper_curse[ID] = false;
			else skull_curse[ID] = false;
			setVeclocity();
			setPosition();
		}
		else {
			oldAction = currentAction = STAY_STILL;
			velX = velY = 0;
			invincible_time = 0;
			setEntitySize(32, 42);
			setPosition((SCREEN_WIDTH - entityWidth()) >> 1, (SCREEN_HEIGHT - entityHeight()) >> 1);
			onGround = jump = invincible = sit = false;
		}
	}
};

//game play
Entity player;
std::vector<Entity> skull(MAX_SKULL), reaper(MAX_REAPER);
#endif // !ENTITY_H
