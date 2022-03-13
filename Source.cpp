#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<fstream>
#include<algorithm>
#include<string>
#include<sstream>
#include<vector>
#include<random>
using namespace std;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int ACCELERATOR = 1;
const int MAX_SKULL = 7;
const int MAX_REAPER = 3;
int identification = 0;
bool EXPLODED = false, SHADOW_CAUGHT = false, DEAD = false;

//random spawn for skull and reaper
vector<bool> skull_curse(MAX_SKULL), reaper_curse(MAX_REAPER);

random_device rd;
mt19937 mt(rd());

//create randomness for spawning
uniform_int_distribution<> curse_construction(1, 1024);
bool skull_curse_construction() {
	Uint16 curse_creation = curse_construction(mt);
	if((curse_creation & 511) == 0) return true;
	return false;
}
bool reaper_curse_construction() {
	Uint16 curse_creation = curse_construction(mt);
	if(curse_creation == 1024) return true;
	return false;
}

//randomly play move sound of bot
uniform_int_distribution<> sound_play(1, 128);

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

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
		if(started) {
			if(paused) return pauseTick;
			else return SDL_GetTicks() - startTick;
		}
		return 0;
	}
	bool isStarted() {
		return started;
	}
	bool isPaused() {
		return started && paused;
	}
};

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
		width = w;
		height = h;
	}
	void setColor(Uint8 red, Uint8 green, Uint8 blue) {
		SDL_SetTextureColorMod(currentTexture, red, green, blue);
	}
	void setBlendMode(SDL_BlendMode blending) {
		SDL_SetTextureBlendMode(currentTexture, blending);
	}
	void setAlpha(Uint8 alpha) {
		SDL_SetTextureAlphaMod(currentTexture, alpha);
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
	MOUSE_DOWN
};
class Button {
	int posX, posY, currentSprite;
	Texture buttonTexture;
public:
	Button() {
		posX = posY = 0;
		currentSprite = MOUSE_OUT;
	}
	void setPosition(int x, int y) {
		posX = x;
		posY = y;
	}
	void handleEvent(SDL_Event* e) {
		if(e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			if(x < posX || x > posX + buttonTexture.getWidth() || y < posY || y > posY + buttonTexture.getHeight()) currentSprite = MOUSE_OUT;
			else switch(e->type) {
				case SDL_MOUSEMOTION: currentSprite = MOUSE_IN; return;
				case SDL_MOUSEBUTTONDOWN: currentSprite = MOUSE_DOWN; return;
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
	void renderButton() {
		buttonTexture.render(posX, posY);
	}
	int getCurrentSprite() {
		return currentSprite;
	}
	int buttonWidth() {
		return buttonTexture.getWidth();
	}
	int buttonHeight() {
		return buttonTexture.getHeight();
	}
	void reset() {
		currentSprite = MOUSE_OUT;
	}
	void free() {
		buttonTexture.free();
	}
};

//support texture for bot
Texture reaper_death_texture, skull_warning, reaper_warning;

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
	vector<vector<SDL_Rect>> entitySpriteClip;
	vector<int> subSprite;

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
				uniform_int_distribution<> Rand(1, 2);
				entityVel = Rand(mt);
			}
			else {
				uniform_int_distribution<> Rand(3, 5);
				entityVel = Rand(mt);
			}
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
			uniform_int_distribution<> vertical(100, 570 - entityTexture.getHeight());
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
			if((leftB < rightA && (leftB + (player.entityWidth() >> 1) >= leftA || leftB + (player.entityWidth() >> 1) <= rightA)) && bottomB > topA && topB < topA && !player.onGround) {
				Mix_HaltChannel(3);
				Mix_PlayChannel(3, die, 0);
				player.velY = -10;
				setVeclocity();
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
		if(!motionDelay) {
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
						posX += entityVel;
						flip = SDL_FLIP_NONE;
					}
					else if(posX > player.posX) {
						posX -= entityVel;
						flip = SDL_FLIP_HORIZONTAL;
					}
					if(posY > player.posY) posY -= entityVel;
					else if(posY < player.posY) posY += entityVel;

					collideRegion.x = posX + 9;
					collideRegion.y = posY + 3;

					if(collided(player)) SHADOW_CAUGHT = DEAD = true;
				}
				else {
					if(create_sound == 128 && !Mix_Playing(4)) {
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
		motionDelay = (motionDelay ? false : true);
	}
	void alert() {
		if(followPlayer) reaper_warning.render((posX < 0 ? 0 : SCREEN_WIDTH - reaper_warning.getWidth()), collideRegion.y - (reaper_warning.getWidth() - collideRegion.h) / 2);
		else skull_warning.render((posX < 0 ? 0 : SCREEN_WIDTH - reaper_warning.getWidth()), collideRegion.y - (reaper_warning.getWidth() - collideRegion.h) / 2);
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

//shared item
enum gameOption {
	MENU,
	PLAY,
	INSTRUCTION,
	HIGH_SCORE
};
Uint8 option = MENU;
Texture transition_screen;
const int BUTTON_WIDTH = 100;
const int BUTTON_HEIGHT = 70;
TTF_Font* button_font;
TTF_Font* deco_font;
Mix_Chunk* touch_button = nullptr;
Mix_Chunk* click_button = nullptr;
Button back;
bool touch_back = false;

//menu
Texture menu_background, menu_title;
Button play, instruction, high_score;
Mix_Music* menu_music = nullptr;
bool touch_play = false, touch_instruction = false, touch_high_score = false, lock_menu_button_sound = false;

//game play
Timer survive_clock;
bool PAUSE = false;
string player_name;
Uint32 survival_time = 0;
Texture game_background, game_map, exploded_texture, shadow_caught_texture, support_board, dead_background;
Entity player;
vector<SDL_Rect> platform(16);
vector<Entity> skull(MAX_SKULL), reaper(MAX_REAPER);
Mix_Chunk* exploded_sound = nullptr;
Mix_Chunk* shadow_caught_sound = nullptr;
Button pause_game, resume_game, exit_game;
bool touch_pause = false, lock_pause_button_sound = false, touch_resume = false, touch_exit = false, lock_resume_and_exit_button_sound = false;

//instruction
Texture instruction_background;
bool lock_instruction_button_sound = false;

//high score
Texture high_score_background, high_score_title;
Button resetScore;
vector<pair<string, Uint32>> score_data;
bool touch_reset = false, lock_high_score_button_sound = false;

void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark forest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
}
void loadMedia() {
	//share items
	//transition screen
	transition_screen.loadFromFile("texture/transition_screen.png");
	transition_screen.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	transition_screen.setBlendMode(SDL_BLENDMODE_BLEND);
	//font
	button_font = TTF_OpenFont("font/Cabanyal-Z.ttf", 50);
	deco_font = TTF_OpenFont("font/ZombieGirlfriend.ttf", 50);
	//touch button sound
	touch_button = Mix_LoadWAV("sound/touch_button.wav");
	Mix_VolumeChunk(touch_button, 100);
	//click button sound
	click_button = Mix_LoadWAV("sound/click_button.wav");
	Mix_VolumeChunk(click_button, 100);
	//back button
	back.textureFromText("Back", button_font, {255,255,255});
	back.setButtonSize(BUTTON_WIDTH * 0.8, BUTTON_HEIGHT * 0.8);
	back.setPosition(25, 25);

	//set up menu region
	//menu background
	menu_background.loadFromFile("texture/menu_background.png");
	menu_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	//game title
	menu_title.loadFromRenderedText("Dark forest", deco_font, {138,3,3});
	menu_title.setSize(550, 175);
	//start button
	play.textureFromText("Play", button_font, {255,255,255});
	play.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	play.setPosition((SCREEN_WIDTH - play.buttonWidth()) / 2, 250);
	//instruction button
	instruction.textureFromText("Instruction", button_font, {255,255,255});
	instruction.setButtonSize(BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	instruction.setPosition((SCREEN_WIDTH - instruction.buttonWidth()) / 2, 370);
	//high score button
	high_score.textureFromText("High score", button_font, {255,255,255});
	high_score.setButtonSize(BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	high_score.setPosition((SCREEN_WIDTH - high_score.buttonWidth()) / 2, 490);
	//menu music
	menu_music = Mix_LoadMUS("sound/menu_music.wav");
	Mix_VolumeMusic(30);

	//set up game play
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
	//bot
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
	skull_warning.loadFromFile("texture/skull_warning.png");
	skull_warning.setSize(50, 43);
	exploded_texture.loadFromFile("texture/exploded.png");
	exploded_texture.setSize(42, 42);
	exploded_sound = Mix_LoadWAV("sound/exploded.wav");
	Mix_VolumeChunk(exploded_sound, 100);
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
		e.setVeclocity();
		e.setPosition();
		e.loadDeadSound("sound/reaper_death.wav", 20);
	}
	identification = 0;
	reaper_warning.loadFromFile("texture/reaper_warning.png");
	reaper_warning.setSize(50, 43);
	shadow_caught_texture.loadFromFile("texture/shadow_caught.png", 0);
	shadow_caught_texture.setSize(42, 42);
	shadow_caught_sound = Mix_LoadWAV("sound/shadow_caught.wav");
	Mix_VolumeChunk(shadow_caught_sound, 100);
	//pause game button
	pause_game.textureFromText("Pause", button_font, {255,255,255});
	pause_game.setButtonSize(BUTTON_WIDTH * 0.8, BUTTON_HEIGHT * 0.8);
	pause_game.setPosition(15, 10);
	//support board
	support_board.loadFromFile("texture/support_board.png");
	support_board.setSize(150, 200);
	//resume game button
	resume_game.textureFromText("Resume", button_font, {255,255,255});
	resume_game.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	resume_game.setPosition((SCREEN_WIDTH - resume_game.buttonWidth()) / 2, 255);
	//exit game button
	exit_game.textureFromText("Exit", button_font, {255,255,255});
	exit_game.setButtonSize(BUTTON_WIDTH * 0.8, BUTTON_HEIGHT);
	exit_game.setPosition((SCREEN_WIDTH - exit_game.buttonWidth()) / 2, 345);
	//dead background
	dead_background.loadFromFile("texture/dead_background.png");
	dead_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	dead_background.setBlendMode(SDL_BLENDMODE_BLEND);

	//set up instruction region
	//instruction background
	instruction_background.loadFromFile("texture/instruction_background.png");
	instruction_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//set up high score region
	//high score background
	high_score_background.loadFromFile("texture/high_score_background.png");
	high_score_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	//high score title
	high_score_title.loadFromRenderedText("high score", deco_font, {125,60,152});
	high_score_title.setSize(250, 100);
	//reset button
	resetScore.textureFromText("Reset", button_font, {255,255,255});
	resetScore.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	resetScore.setPosition((SCREEN_WIDTH - 25 - resetScore.buttonWidth()), 25);
}
void close() {
	//close shared items
	transition_screen.free();
	TTF_CloseFont(button_font);
	TTF_CloseFont(deco_font);
	button_font = deco_font = nullptr;
	Mix_FreeChunk(touch_button);
	Mix_FreeChunk(click_button);
	touch_button = click_button = nullptr;
	back.free();

	//close menu
	menu_background.free();
	menu_title.free();
	play.free();
	instruction.free();
	high_score.free();
	Mix_FreeMusic(menu_music);
	menu_music = nullptr;

	//close game play
	game_background.free();
	game_map.free();
	support_board.free();
	dead_background.free();
	platform.resize(0);

	player.free();

	skull_warning.free();
	for(auto& e : skull) e.free();
	skull.resize(0);
	skull_curse.resize(0);
	exploded_texture.free();
	Mix_FreeChunk(exploded_sound);
	exploded_sound = nullptr;

	reaper_warning.free();
	reaper_death_texture.free();
	for(auto& e : reaper) e.free();
	reaper.resize(0);
	reaper_curse.resize(0);
	shadow_caught_texture.free();
	Mix_FreeChunk(shadow_caught_sound);
	shadow_caught_sound = nullptr;

	pause_game.free();
	resume_game.free();
	exit_game.free();

	//close instruction
	instruction_background.free();

	//close high score
	ofstream new_data("high_score_data.txt");
	for(auto& sd : score_data) if(sd.first != "") new_data << sd.first << ' ' << sd.second << "\n";
	score_data.resize(0);
	high_score_background.free();
	high_score_title.free();
	resetScore.free();

	//close renderer and window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = nullptr;
	window = nullptr;

	//close all tool
	IMG_Quit();
	TTF_Quit();
	Mix_Quit();
	SDL_Quit();
}

void resetGame() {
	for(int i = 0; i < MAX_SKULL; i++) {
		skull_curse[i] = false;
		skull[i].setVeclocity();
		skull[i].setPosition();
		skull[i].resetEntity();
	}

	for(int i = 0; i < MAX_REAPER; i++) {
		reaper_curse[i] = false;
		reaper[i].setVeclocity();
		reaper[i].setPosition();
		reaper[i].resetEntity();
	}

	player.setPosition((SCREEN_WIDTH - player.entityWidth()) / 2, (SCREEN_HEIGHT - player.entityHeight()) / 2);
	player.resetEntity();

	player_name = "";
	survival_time = 0;
	PAUSE = EXPLODED = SHADOW_CAUGHT = DEAD = false;
}

void updateScoreData() {
	if(score_data.size() != 0) {
		if(player_name != "") {
			score_data.emplace_back(player_name, survival_time);
			sort(begin(score_data), end(score_data), [](pair<string, Uint32> a, pair<string, Uint32> b) {return a.second > b.second; });
			score_data.resize(5);
		}
	}
	else {
		ifstream old_data("high_score_data.txt");
		string data_name;
		Uint32 data_time;
		while(!old_data.eof()) {
			old_data >> data_name >> data_time;
			score_data.emplace_back(data_name, data_time);
		}
		score_data.pop_back();
		score_data.resize(5);
	}
}

string timeFormat(Uint32 time) {
	time /= 1000;
	string min = to_string(time / 60), sec = to_string(time % 60);
	if(min.length() < 2) min = '0' + min;
	if(sec.length() < 2) sec = '0' + sec;
	return min + ':' + sec;
}

void showTime() {
	stringstream timeText;
	timeText.str("");
	timeText << timeFormat(survive_clock.getTick());
	Texture timeTexture;
	timeTexture.loadFromRenderedText(timeText.str(), deco_font, {255,255,0});
	timeTexture.setSize(90, 45);
	timeTexture.render((SCREEN_WIDTH - timeTexture.getWidth()) / 2, 10);
	timeTexture.free();
}

int main(int argc, char** argv) {
	init();
	loadMedia();
	updateScoreData();
	SDL_Event e;
	while(true) switch(option) {
		//menu part
		case MENU:
			if(Mix_PlayingMusic() == 0) Mix_PlayMusic(menu_music, 1);
			while(SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) {
					close(); return 0;
				}
				play.handleEvent(&e);
				instruction.handleEvent(&e);
				high_score.handleEvent(&e);
			}
			switch(play.getCurrentSprite()) {
				case MOUSE_OUT:
					touch_play = false;
					play.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_play = true;
					play.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					option = PLAY;
					play.reset();
					Mix_PlayChannel(1, click_button, 0);
					break;
			}
			switch(instruction.getCurrentSprite()) {
				case MOUSE_OUT:
					touch_instruction = false;
					instruction.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_instruction = true;
					instruction.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					option = INSTRUCTION;
					instruction.reset();
					Mix_PlayChannel(1, click_button, 0);
					break;
			}
			switch(high_score.getCurrentSprite()) {
				case MOUSE_OUT:
					touch_high_score = false;
					high_score.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_high_score = true;
					high_score.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					option = HIGH_SCORE;
					high_score.reset();
					Mix_PlayChannel(1, click_button, 0);
					break;
			}
			if(option == MENU) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				menu_background.render(0, 0);
				menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) / 2, 25);
				play.renderButton();
				instruction.renderButton();
				high_score.renderButton();
				SDL_RenderPresent(renderer);
				if(touch_play || touch_instruction || touch_high_score) {
					if(!lock_menu_button_sound) {
						lock_menu_button_sound = true;
						Mix_PlayChannel(1, touch_button, 0);
					}
				}
				else lock_menu_button_sound = false;
			}
			break;

		//game play part
		case PLAY:
			resetGame();
			pause_game.setButtonColor(255, 255, 255);

			Mix_FadeOutMusic(1500);

			for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close();
					return 0;
				}

				transition_screen.setAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				menu_background.render(0, 0);
				menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) / 2, 25);

				play.renderButton();
				instruction.renderButton();
				high_score.renderButton();

				transition_screen.render(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close();
					return 0;
				}

				transition_screen.setAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				game_background.render(0, 0);
				game_map.render(0, 0);

				player.renderEntity();

				pause_game.renderButton();

				showTime();

				transition_screen.render(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			while(!DEAD && option == PLAY) {
				if(!survive_clock.isStarted()) survive_clock.start();
				if(PAUSE) {
					survive_clock.pause();

					while(SDL_PollEvent(&e) != 0) {
						if(e.type == SDL_QUIT) {
							close(); return 0;
						}
						resume_game.handleEvent(&e);
						exit_game.handleEvent(&e);
					}

					switch(resume_game.getCurrentSprite()) {
						case MOUSE_OUT:
							touch_resume = false;
							resume_game.setButtonColor(255, 255, 255);
							break;
						case MOUSE_IN:
							touch_resume = true;
							resume_game.setButtonColor(255, 0, 0);
							break;
						case MOUSE_DOWN:
							PAUSE = false;
							resume_game.reset();
							Mix_PlayChannel(1, click_button, 0);
							break;
					}

					switch(exit_game.getCurrentSprite()) {
						case MOUSE_OUT:
							touch_exit = false;
							exit_game.setButtonColor(255, 255, 255);
							break;
						case MOUSE_IN:
							touch_exit = true;
							exit_game.setButtonColor(255, 0, 0);
							break;
						case MOUSE_DOWN:
							option = MENU;
							exit_game.reset();
							Mix_PlayChannel(1, click_button, 0);
							break;
					}

					if(PAUSE && option == PLAY) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}
						player.renderEntity();

						support_board.render((SCREEN_WIDTH - support_board.getWidth()) / 2, (SCREEN_HEIGHT - support_board.getHeight()) / 2);
						resume_game.renderButton();
						exit_game.renderButton();

						showTime();

						SDL_RenderPresent(renderer);

						if(touch_resume || touch_exit) {
							if(!lock_resume_and_exit_button_sound) {
								lock_resume_and_exit_button_sound = true;
								Mix_PlayChannel(1, touch_button, 0);
							}
						}
						else lock_resume_and_exit_button_sound = false;
					}
				}
				else {
					survive_clock.resume();

					while(SDL_PollEvent(&e) != 0) {
						if(e.type == SDL_QUIT) {
							close(); return 0;
						}
						pause_game.handleEvent(&e);
						player.handleEvent(&e);
					}

					switch(pause_game.getCurrentSprite()) {
						case MOUSE_OUT:
							touch_pause = false;
							pause_game.setButtonColor(255, 255, 255);
							break;
						case MOUSE_IN:
							touch_pause = true;
							pause_game.setButtonColor(255, 0, 0);
							break;
						case MOUSE_DOWN:
							PAUSE = true;
							pause_game.reset();
							Mix_PlayChannel(1, click_button, 0);
							break;
					}

					if(!PAUSE) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);
						pause_game.renderButton();

						showTime();

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) {
								skull[i].move(platform, player);
								skull[i].renderEntity();
							}
							else {
								skull[i].alert();
								skull_curse[i] = skull_curse_construction();
							}
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) {
								reaper[i].move(platform, player);
								reaper[i].renderEntity();
							}
							else {
								reaper[i].alert();
								reaper_curse[i] = reaper_curse_construction();
							}
						}
						player.move(platform, player);
						player.renderEntity();

						SDL_RenderPresent(renderer);

						if(touch_pause) {
							if(!lock_pause_button_sound) {
								lock_pause_button_sound = true;
								Mix_PlayChannel(1, touch_button, 0);
							}
						}
						else lock_pause_button_sound = false;
					}
				}
			}

			survive_clock.pause();

			if(option == PLAY) {
				if(SHADOW_CAUGHT) {
					Mix_HaltChannel(-1);
					Mix_PlayChannel(3, shadow_caught_sound, 0);
					for(int i = 0; i < 12; i++) {
						if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
							close(); return 0;
						}

						SDL_Rect shadowSprite = {256 * i,0,256,256};

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);

						player.renderEntity();

						shadow_caught_texture.render(player.entityPosition().x - 5, player.entityPosition().y, &shadowSprite);

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}

						showTime();

						SDL_RenderPresent(renderer);
						SDL_Delay(500);
					}
					while(Mix_Playing(3)) if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close();
						return 0;
					}
				}
				else if(EXPLODED) {
					Mix_HaltChannel(-1);
					Mix_PlayChannel(4, exploded_sound, 0);
					for(int i = 0; i < 14; i++) {
						if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
							close(); return 0;
						}

						SDL_Rect explodeSprite = {128 * i,0,128,128};

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);

						exploded_texture.render(player.entityPosition().x - 5, player.entityPosition().y, &explodeSprite);

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}

						showTime();

						SDL_RenderPresent(renderer);
						SDL_Delay(150);
					}
					while(Mix_Playing(4)) if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close();
						return 0;
					}
				}

				survival_time = survive_clock.getTick();

				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close();
						return 0;
					}

					dead_background.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.render(0, 0);
					game_map.render(0, 0);

					if(SHADOW_CAUGHT) {
						SDL_Rect sprite = {2816,0,256,256};
						shadow_caught_texture.render(player.entityPosition().x - 5, player.entityPosition().y, &sprite);
					}
					else {
						SDL_Rect sprite = {1664,0,128,128};
						exploded_texture.render(player.entityPosition().x - 5, player.entityPosition().y, &sprite);
					}

					for(int i = 0; i < MAX_SKULL; i++) {
						if(skull_curse[i]) skull[i].renderEntity();
						else skull[i].alert();
					}
					for(int i = 0; i < MAX_REAPER; i++) {
						if(reaper_curse[i]) reaper[i].renderEntity();
						else reaper[i].alert();
					}

					showTime();

					dead_background.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close();
						return 0;
					}

					dead_background.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					dead_background.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				bool confirmedName = false;
				SDL_StartTextInput();
				while(!confirmedName) {
					while(SDL_PollEvent(&e) != 0) switch(e.type) {
						case SDL_QUIT:
							close(); return 0;
						case SDL_KEYDOWN:
							switch(e.key.keysym.sym) {
								case SDLK_BACKSPACE:
									if(player_name.length() > 0) player_name.pop_back();
									break;
								case SDLK_c:
									if(SDL_GetModState() && KMOD_CTRL) SDL_SetClipboardText(player_name.c_str());
									break;
								case SDLK_x:
									if(SDL_GetModState() && KMOD_CTRL) {
										SDL_SetClipboardText(player_name.c_str());
										player_name = "";
									}
									break;
								case SDLK_v:
									if(SDL_GetModState() && KMOD_CTRL) player_name += SDL_GetClipboardText();
									break;
								case SDLK_RETURN:
									confirmedName = true;
									break;
							}
							break;
						case SDL_TEXTINPUT:
							if(!(SDL_GetModState() && KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'x' || e.text.text[0] == 'X' || e.text.text[0] == 'v' || e.text.text[0] == 'V'))) player_name += e.text.text;
							break;
					}
					if(!confirmedName) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						if(player_name == "") player_name = " ";
						Texture nameTexture, instrucTexture;

						instrucTexture.loadFromRenderedText("Enter your name", deco_font, {255,0,0});
						instrucTexture.setSize(450, 100);
						instrucTexture.render((SCREEN_WIDTH - instrucTexture.getWidth()) / 2, 210);

						nameTexture.loadFromRenderedText(player_name, deco_font, {91,44,111});
						nameTexture.setSize(30 * (int)player_name.length(), 100);
						nameTexture.render((SCREEN_WIDTH - nameTexture.getWidth()) / 2, 330);

						SDL_RenderPresent(renderer);

						if(player_name == " ") player_name = "";
						nameTexture.free();
						instrucTexture.free();
					}
				}
				SDL_StopTextInput();

				while(!player_name.empty() && player_name[0] == ' ') player_name.erase(0, 1);
				while(!player_name.empty() && player_name.back() == ' ') player_name.pop_back();

				updateScoreData();

				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					if(player_name == "") player_name = " ";
					Texture textTexture, instrucTexture;

					instrucTexture.loadFromRenderedText("Enter your name", deco_font, {255,0,0});
					instrucTexture.setSize(450, 100);
					instrucTexture.render((SCREEN_WIDTH - instrucTexture.getWidth()) / 2, 210);

					textTexture.loadFromRenderedText(player_name, deco_font, {91,44,111});
					textTexture.setSize(30 * (int)player_name.length(), 100);
					textTexture.render((SCREEN_WIDTH - textTexture.getWidth()) / 2, 330);

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					if(player_name == " ") player_name = "";
					textTexture.free();
					instrucTexture.free();

					SDL_Delay(50);
				}
				SDL_Delay(500);

				play.setButtonColor(255, 255, 255);
				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					menu_background.render(0, 0);
					menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) / 2, 25);

					play.renderButton();
					instruction.renderButton();
					high_score.renderButton();

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				option = MENU;
			}
			else {
				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.render(0, 0);
					game_map.render(0, 0);

					for(int i = 0; i < MAX_SKULL; i++) {
						if(skull_curse[i]) skull[i].renderEntity();
						else skull[i].alert();
					}
					for(int i = 0; i < MAX_REAPER; i++) {
						if(reaper_curse[i]) reaper[i].renderEntity();
						else reaper[i].alert();
					}
					player.renderEntity();

					showTime();

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				play.setButtonColor(255, 255, 255);
				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					menu_background.render(0, 0);
					menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) / 2, 25);

					play.renderButton();
					instruction.renderButton();
					high_score.renderButton();

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);
			}

			survive_clock.stop();
			break;

		//instruction part
		case INSTRUCTION:
			while(SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) {
					close(); return 0;
				}
				back.handleEvent(&e);
			}
			switch(back.getCurrentSprite()) {
				case MOUSE_OUT:
					touch_back = false;
					back.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_back = true;
					back.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					option = MENU;
					back.reset();
					Mix_PlayChannel(1, click_button, 0);
					break;
			}
			if(option == INSTRUCTION) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				instruction_background.render(0, 0);
				back.renderButton();
				SDL_RenderPresent(renderer);
				if(touch_back) {
					if(!lock_instruction_button_sound) {
						lock_instruction_button_sound = true;
						Mix_PlayChannel(1, touch_button, 0);
					}
				}
				else lock_instruction_button_sound = false;
			}
			break;

		//high score part
		case HIGH_SCORE:
			while(SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) {
					close(); return 0;
				}
				back.handleEvent(&e);
				resetScore.handleEvent(&e);
			}
			switch(back.getCurrentSprite()) {
				case MOUSE_OUT:
					touch_back = false;
					back.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_back = true;
					back.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					option = MENU;
					back.reset();
					Mix_PlayChannel(1, click_button, 0);
					break;
			}
			switch(resetScore.getCurrentSprite()) {
				case MOUSE_OUT:
					touch_reset = false;
					resetScore.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_reset = true;
					resetScore.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					score_data.clear();
					score_data.resize(5);
					resetScore.reset();
					Mix_PlayChannel(1, click_button, 0);
					break;
			}
			if(option == HIGH_SCORE) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				high_score_background.render(0, 0);
				high_score_title.render((SCREEN_WIDTH - high_score_title.getWidth()) / 2, 75);
				back.renderButton();
				resetScore.renderButton();
				for(int i = 0; i < score_data.size(); i++) {
					stringstream text;
					text.str("");
					text << (score_data[i].second != 0 ? score_data[i].first : "none") << string(5, ' ') << timeFormat(score_data[i].second);
					SDL_Color text_color;
					switch(i) {
						case 0: text_color = {255,215,0}; break;
						case 1:	text_color = {192,192,192}; break;
						default: text_color = {205, 127, 50}; break;
					}
					Texture data_information;
					data_information.loadFromRenderedText(text.str(), deco_font, text_color);
					data_information.setSize(20 * (int)text.str().length(), 75);
					data_information.render((SCREEN_WIDTH - data_information.getWidth()) / 2, 200 + i * 75);
					data_information.free();
				}
				SDL_RenderPresent(renderer);
				if(touch_back || touch_reset) {
					if(!lock_high_score_button_sound) {
						lock_high_score_button_sound = true;
						Mix_PlayChannel(1, touch_button, 0);
					}
				}
				else lock_high_score_button_sound = false;
			}
			break;
	}
}
