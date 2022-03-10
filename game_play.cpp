//https://TOKEN@github.com/USERNAME/REPO.git

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<random>
using namespace std;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int ACCELERATOR = 1;
const int MAX_SKULL = 5;
const int MAX_REAPER = 3;
int identification = 0;
bool MENU, PLAY, INSTRUCTION, HIGH_SCORE, EXPLODED, SHADOW_CAUGHT, DEAD;

vector<bool> skull_curse(MAX_SKULL), reaper_curse(MAX_REAPER);

random_device rd;
mt19937 mt(rd());

uniform_int_distribution<> curse_construction(1, 1024);
bool skull_curse_construction() {
	Uint16 curse_creation = curse_construction(mt);
	if((curse_creation & 255) == 0) return true;
	return false;
}
bool reaper_curse_construction() {
	Uint16 curse_creation = curse_construction(mt);
	if(curse_creation == 1024) return true;
	return false;
}

uniform_int_distribution<> sound_play(1, 128);

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

class Texture {
	SDL_Texture* currentTexture;
	int width, height;
public:
	Texture() {
		currentTexture = nullptr;
		width = height = 0;
	}
	void free() {
		SDL_DestroyTexture(currentTexture);
		currentTexture = nullptr;
		width = height = 0;
	}
	void loadFromFile(string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		free();
		SDL_Surface* loadedImage = IMG_Load(path.c_str());
		SDL_SetColorKey(loadedImage, SDL_TRUE, SDL_MapRGB(loadedImage->format, r, g, b));
		currentTexture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		width = loadedImage->w;
		height = loadedImage->h;
		SDL_FreeSurface(loadedImage);
		loadedImage = nullptr;
	}
	void loadFromRenderedText(string textureText, TTF_Font* font, SDL_Color textColor) {
		free();
		SDL_Surface* loadedText = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
		currentTexture = SDL_CreateTextureFromSurface(renderer, loadedText);
		width = loadedText->w;
		height = loadedText->h;
		SDL_FreeSurface(loadedText);
		loadedText = nullptr;
	}
	void setSize(int w, int h) {
		width = w, height = h;
	}
	void setColor(Uint8 red, Uint8 green, Uint8 blue) {
		SDL_SetTextureColorMod(currentTexture, red, green, blue);
	}
	void render(int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) {
		SDL_Rect renderQuad{x,y,width,height};
		SDL_RenderCopyEx(renderer, currentTexture, clip, &renderQuad, angle, center, flip);
	}
	int getWidth() { return width; }
	int getHeight() { return height; }
};

enum ButtonSprite {
	MOUSE_OUT,
	MOUSE_IN,
	MOUSE_DOWN,
	MOUSE_UP,
	MOUSE_TOTAL
};
class Button {
	int posX, posY, currentSprite;
	vector<SDL_Rect> buttonSpriteClip;
	Texture buttonTexture;
public:
	Button() {
		buttonSpriteClip.resize(MOUSE_TOTAL);
		posX = posY = 0;
		currentSprite = MOUSE_OUT;
	}
	void setPosition(int x, int y) {
		posX = x;
		posY = y;
	}
	void handleEvent(SDL_Event* e) {
		if(e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			bool inside = true;
			if(x < posX || x > posX + buttonTexture.getWidth() || y < posY || y > posY + buttonTexture.getHeight()) inside = false;
			if(!inside) currentSprite = MOUSE_OUT;
			else {
				switch(e->type) {
					case SDL_MOUSEMOTION:
						currentSprite = MOUSE_IN;
						break;
					case SDL_MOUSEBUTTONDOWN:
						currentSprite = MOUSE_DOWN;
						break;
					case SDL_MOUSEBUTTONUP:
						currentSprite = MOUSE_UP;
						break;
				}
			}
		}
	}
	void textureFromFile(string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		buttonTexture.loadFromFile(path, r, g, b);
	}
	void textureFromText(string text, TTF_Font* font, SDL_Color color) {
		buttonTexture.loadFromRenderedText(text, font, color);
	}
	void setButtonSize(int w, int h) {
		buttonTexture.setSize(w, h);
	}
	void setButtonColor(Uint8 red, Uint8 green, Uint8 blue) {
		buttonTexture.setColor(red, green, blue);
	}
	void setSprite(int sprite, int sprite_x, int sprite_y, int sprite_width, int sprite_height) {
		buttonSpriteClip[sprite] = {sprite_x,sprite_y,sprite_width,sprite_height};
	}
	void render() {
		buttonTexture.render(posX, posY, &buttonSpriteClip[currentSprite]);
	}
	int getSprite() {
		return currentSprite;
	}
	int buttonWidth() {
		return buttonTexture.getWidth();
	}
	int buttonHeight() {
		return buttonTexture.getHeight();
	}
	void free() {
		buttonSpriteClip.resize(0);
		buttonTexture.free();
	}
};

class Timer {
	Uint32 startTick, pauseTick;
	bool started, paused;
public:
	Timer() {
		startTick = pauseTick = 0;
		started = paused = false;
	}
	void start() {
		startTick = SDL_GetTicks();
		started = true;
		pauseTick = 0;
		paused = false;
	}
	void stop() {
		startTick = pauseTick = 0;
		started = paused = false;
	}
	void pause() {
		if(started && !paused) {
			paused = true;
			pauseTick = SDL_GetTicks() - startTick;
			startTick = 0;
		}
	}
	void resume() {
		if(started && paused) {
			paused = false;
			startTick = SDL_GetTicks() - pauseTick;
			pauseTick = 0;
		}
	}
	Uint32 getTick() {
		Uint32 time = 0;
		if(started) {
			if(paused) time = pauseTick;
			else time = SDL_GetTicks() - startTick;
		}
		return time;
	}
	bool isStarted() {
		return started;
	}
	bool isPaused() {
		return started && paused;
	}
};

Texture reaper_death_texture;

enum entityMovement {
	STAY_STILL,
	MOVE_LEFT,
	MOVE_RIGHT,
	JUMP,
	SIT,
	TOTAL_MOVEMENT
};
class Entity {
	int ID, posX, posY, velX, velY, entityVel, currentSprite, currenSubSprite, motionDelay;
	bool autonomous, followPlayer, onGround, sit;

	Texture entityTexture;
	SDL_Rect collideRegion;
	SDL_RendererFlip flip;
	vector<vector<SDL_Rect>> entitySpriteClip;
	vector<int> subSprite;

	Mix_Chunk* moveAround;
	Mix_Chunk* die;
public:
	Entity() {
		ID = 0;
		motionDelay = 0;
		posX = posY = 0;
		velX = velY = entityVel = 0;
		collideRegion = {0,0,0,0};

		currenSubSprite = 0;
		currentSprite = STAY_STILL;

		moveAround = die = nullptr;

		autonomous = followPlayer = onGround = sit = false;
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
			uniform_int_distribution<> Rand(3, 5);
			entityVel = Rand(mt);
		}
		else entityVel = v;
	}
	void loadEntityTexture(string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
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
			uniform_int_distribution<> horizontal(0, 1);
			bool left = horizontal(mt);
			if(left) {
				posX = -entityTexture.getWidth();
				flip = SDL_FLIP_NONE;
			}
			else {
				posX = SCREEN_WIDTH;
				flip = SDL_FLIP_HORIZONTAL;
			}
			uniform_int_distribution<> vertical(0, 570 - entityTexture.getHeight());
			posY = vertical(mt);
			if(!followPlayer) collideRegion = {posX + 17,posY + 55,30,30};
			else collideRegion = {posX + 9,posY + 3,32,45};
		}
		else {
			collideRegion.x = posX = x;
			collideRegion.y = posY = y;
		}
	}
	void loadMoveSound(string path, Uint8 volume) {
		Mix_FreeChunk(moveAround);
		moveAround = nullptr;
		moveAround = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(moveAround, volume);
	}
	void loadDeadSound(string path, Uint8 volume) {
		Mix_FreeChunk(die);
		die = nullptr;
		die = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(die, volume);
	}
	void renderEntity() {
		entityTexture.render(posX, posY, &entitySpriteClip[currentSprite][currenSubSprite], 0.0, nullptr, flip);
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
					if(onGround) {
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
						entityTexture.setSize(entityWidth(), entityHeight() * 3 / 2);
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
		if(currentSprite != oldSprite) currenSubSprite = -1;
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
			if((leftB < rightA && (leftB + (player.entityWidth() >> 1) >= leftA || leftB + (player.entityWidth() >> 1) <= rightA)) &&
			   bottomB > topA &&
			   topB < topA &&
			   !player.onGround &&
			   player.velY > 0) {
				Mix_HaltChannel(3);
				Mix_PlayChannel(3, die, 0);
				player.velY = -10;
				setPosition();
				reaper_curse[ID] = reaper_curse_construction();
				reaper_death_texture.render(leftA, topA);
				SDL_RenderPresent(renderer);
				SDL_Delay(50);
				return false;
			}
		}
		return true;
	}
	void move(vector<SDL_Rect>& platform, Entity& player) {
		if(motionDelay == 0) {
			//for bot movement
			if(autonomous) {
				Uint16 create_sound = sound_play(mt);

				if(followPlayer) {
					if(create_sound == 128 && !Mix_Playing(3)) {
						uniform_int_distribution<> file(1, 3);
						Uint16 chosen_sound = file(mt);
						string path = "sound/reaper_move" + to_string(chosen_sound) + ".wav";
						loadMoveSound(path, 20);
						Mix_PlayChannel(3, moveAround, 0);
					}

					if(posX < player.posX) {
						posX++;
						flip = SDL_FLIP_NONE;
					}
					else if(posX > player.posX) {
						posX--;
						flip = SDL_FLIP_HORIZONTAL;
					}
					if(posY > player.posY) posY--;
					else if(posY < player.posY) posY++;

					collideRegion.x = posX + 9;
					collideRegion.y = posY + 3;

					if(collided(player)) {
						setPosition();
						reaper_curse[ID] = reaper_curse_construction();
						//SHADOW_CAUGHT = DEAD = true;
					}
				}
				else {
					if((create_sound & 63) == 0 && !Mix_Playing(4)) {
						uniform_int_distribution<> file(1, 3);
						Uint16 chosen_sound = file(mt);
						string path = "sound/skull_move" + to_string(chosen_sound) + ".wav";
						loadMoveSound(path, 30);
						Mix_PlayChannel(4, moveAround, 0);
					}
					posX += (flip == SDL_FLIP_NONE ? entityVel : -entityVel);

					collideRegion.x = posX + 17;

					if((flip == SDL_FLIP_NONE && posX > SCREEN_WIDTH) || (flip == SDL_FLIP_HORIZONTAL && posX < -entityTexture.getWidth())) {
						setVeclocity();
						setPosition();
						skull_curse[ID] = skull_curse_construction();
					}

					if(collided(player)) {
						setVeclocity();
						setPosition();
						skull_curse[ID] = skull_curse_construction();
						//EXPLODED = DEAD = true;
					}
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
						if(bottomA >= topB && topA + (entityTexture.getHeight() >> 2) < topB) {
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
		motionDelay = (motionDelay + 1) & 1;
	}
};

Texture game_background, game_map;
Entity player;
vector<SDL_Rect> platform(17);
vector<Entity> skull(MAX_SKULL), reaper(MAX_REAPER);

void init() {
	for(auto i : skull_curse) i = skull_curse_construction();
	for(auto i : reaper_curse) i = reaper_curse_construction();
	MENU = true;
	PLAY = INSTRUCTION = HIGH_SCORE = EXPLODED = SHADOW_CAUGHT = DEAD = false;
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark forest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}
void loadMedia() {
	//map texture
	game_map.loadFromFile("texture/game_map.png");
	game_map.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	//background texture
	game_background.loadFromFile("texture/game_background.png");
	game_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//player texture
	player.loadEntityTexture("texture/player.png");
	//player animation
	player.setAmountOfSprite(STAY_STILL, 35);
	for(int i = 0; i < player.getNumberSubSprite(STAY_STILL); i++) player.setEntitySprite(STAY_STILL, i, 96 * (i / 5) + 32, 84 + 42, 32, 42);
	player.setAmountOfSprite(MOVE_RIGHT, 40);
	for(int i = 0; i < player.getNumberSubSprite(MOVE_RIGHT); i++) player.setEntitySprite(MOVE_RIGHT, i, 96 * (i / 5) + 32, 252 + 42, 32, 42);
	player.setAmountOfSprite(MOVE_LEFT, 40);
	for(int i = 0; i < player.getNumberSubSprite(MOVE_LEFT); i++) player.setEntitySprite(MOVE_LEFT, i, 96 * (i / 5) + 32, 252 + 42, 32, 42);
	player.setAmountOfSprite(JUMP, 1);
	player.setEntitySprite(JUMP, 0, 96 + 32, 336 + 40, 32, 44);
	player.setAmountOfSprite(SIT, 30);
	for(int i = 0; i < player.getNumberSubSprite(SIT); i++) player.setEntitySprite(SIT, i, 96 * (i / 5) + 32, 756 + 56, 32, 28);
	//player size
	player.setEntitySize(32, 42);
	//start position
	player.setPosition((SCREEN_WIDTH - player.entityWidth()) / 2, (SCREEN_HEIGHT - player.entityHeight()) / 2);
	//player velocity
	player.setVeclocity(4);
	//player move sound
	player.loadMoveSound("sound/player_move.wav", MIX_MAX_VOLUME);

	//all platform to jump on
	platform[0] = {0,575,960,65};
	platform[1] = {465,529,31,40};
	platform[2] = {349,461,70,17};
	platform[3] = {618,461,48,19};
	platform[4] = {467,371,123,15};
	platform[5] = {215,369,88,17};
	platform[6] = {63,416,106,15};
	platform[7] = {24,324,49,15};
	platform[8] = {370,301,49,15};
	platform[9] = {660,301,103,15};
	platform[10] = {793,234,65,15};
	platform[11] = {466,232,47,18};
	platform[12] = {180,233,121,16};
	platform[13] = {63,165,84,16};
	platform[14] = {525,163,103,18};
	platform[15] = {695,120,87,15};
	platform[16] = {850,73,87,16};

	//bot texture
	//skull missile
	for(auto& e : skull) {
		e.setID(identification);
		identification++;
		e.setAuto();
		e.loadEntityTexture("texture/skull.png");
		e.setAmountOfSprite(STAY_STILL, 320);
		for(int i = 0; i < e.getNumberSubSprite(STAY_STILL); i++) e.setEntitySprite(STAY_STILL, i, 64 * (i / 10), 0, 64, 90);
		e.setEntitySize(64, 90);
		e.setVeclocity();
		e.setPosition();
	}
	identification = 0;
	//grim reaper
	reaper_death_texture.loadFromFile("texture/reaper_death.png");
	reaper_death_texture.setSize(48, 48);
	for(auto& e : reaper) {
		e.setID(identification);
		identification++;
		e.setAuto();
		e.setFollowPlayer();
		e.loadEntityTexture("texture/reaper.png");
		e.setAmountOfSprite(STAY_STILL, 90);
		for(int i = 0; i < e.getNumberSubSprite(STAY_STILL); i++) e.setEntitySprite(STAY_STILL, i, 48 * (i / 10), 0, 48, 48);
		e.setEntitySize(48, 48);
		e.setPosition();
		e.loadDeadSound("sound/reaper_death.wav", 20);
	}
	identification = 0;
}
void close() {
	game_background.free();
	game_map.free();

	player.free();
	platform.resize(0);

	for(auto& e : skull) e.free();
	skull.resize(0);
	skull_curse.resize(0);

	reaper_death_texture.free();
	for(auto& e : reaper) e.free();
	reaper.resize(0);
	reaper_curse.resize(0);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = nullptr;
	window = nullptr;

	IMG_Quit();
	TTF_Quit();
	Mix_Quit();
	SDL_Quit();
}

int main(int argc, char** argv) {
	init();
	loadMedia();
	bool quit = false;
	SDL_Event e;
	stringstream text;
	while(!quit) {
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT) quit = true;
			player.handleEvent(&e);
		}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		game_background.render(0, 0);
		game_map.render(0, 0);

		//SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//for(auto& P : platform) {
		//	SDL_RenderDrawRect(renderer, &P);
		//}

		for(int i = 0; i < MAX_SKULL; i++) {
			if(skull_curse[i]) {
				skull[i].move(platform, player);
				skull[i].renderEntity();
			}
			else skull_curse[i] = skull_curse_construction();
		}

		for(int i = 0; i < MAX_REAPER; i++) {
			if(reaper_curse[i]) {
				reaper[i].move(platform, player);
				reaper[i].renderEntity();
			}
			else reaper_curse[i] = reaper_curse_construction();
		}

		player.move(platform, player);
		player.renderEntity();

		SDL_RenderPresent(renderer);
	}
	close();
	return 0;
}