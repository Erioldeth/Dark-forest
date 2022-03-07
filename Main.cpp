//https://TOKEN@github.com/USERNAME/REPO.git

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>
#include<random>
#include<cmath>
using namespace std;

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 640;
const int ACCELERATOR = 1;
const int MAX_MISSILE = 4;
const int MAX_REAPER = 5 - MAX_MISSILE;
bool MENU, PLAY, INSTRUCTION, HIGH_SCORE, EXPLODED, SHADOW_CAUGHT, PAUSE, DEAD;

random_device rd;
mt19937 mt(rd());

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
	MOUSE_DOWN,
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
	void setSprite(int sprite, int sprite_x, int sprite_y, int sprite_width, int sprite_height) {
		buttonSpriteClip[sprite] = {sprite_x,sprite_y,sprite_width,sprite_height};
	}
	void render() {
		buttonTexture.render(posX, posY, &buttonSpriteClip[currentSprite]);
	}
	void setSpriteState(int state) {
		currentSprite = state;
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

enum entityMovement {
	STAY_STILL,
	MOVE_LEFT,
	MOVE_RIGHT,
	JUMP,
	TOTAL_MOVEMENT
};
class Entity {
	int posX, posY, velX, velY, entityVel, currentSprite, currenSubSprite;
	bool autonomous, followPlayer, onGround;

	Texture entityTexture;
	SDL_RendererFlip flip;
	vector<vector<SDL_Rect>> entitySpriteClip;
	vector<int> subSprite;

	Mix_Chunk* moveAround;
	Mix_Chunk* die;
public:
	Entity() {
		posX = posY = 0;
		velX = velY = entityVel = 0;
		currentSprite = STAY_STILL;
		moveAround = die = nullptr;
		autonomous = followPlayer = onGround = false;
		flip = SDL_FLIP_NONE;
		currenSubSprite = 0;
		subSprite.resize(TOTAL_MOVEMENT);
		entitySpriteClip.resize(TOTAL_MOVEMENT);
	}
	void setAuto() {
		autonomous = true;
	}
	void setFollowPlayer() {
		followPlayer = true;
	}
	void setVeclocity(int v = 0) {
		if(autonomous) {
			uniform_int_distribution<> Rand(2, 4);
			entityVel = Rand(mt);
		}
		else entityVel = v;
	}
	void loadEntityTexture(string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		entityTexture.loadFromFile(path, r, g, b);
	}
	void setEntityColor(Uint8 r, Uint8 g, Uint8 b) {
		entityTexture.setColor(r, g, b);
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
		}
		else {
			posX = x;
			posY = y;
		}
	}
	void loadMoveSound(string path) {
		moveAround = Mix_LoadWAV(path.c_str());
		Mix_VolumeChunk(moveAround, 16);
	}
	void loadDeadSound(string path) {
		die = Mix_LoadWAV(path.c_str());
	}
	void renderEntity() {
		entityTexture.render(posX, posY, &entitySpriteClip[currentSprite][currenSubSprite], 0.0, nullptr, flip);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_Rect playerSquare = {posX,posY,entityTexture.getWidth(),entityTexture.getHeight()};
		//SDL_RenderDrawRect(renderer, &playerSquare);
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
		//reset animation frame
		if(currentSprite != oldSprite) currenSubSprite = -1;
	}
	bool collided(Entity& other) {
		int leftA = posX + entityTexture.getWidth() / 10,
			rightA = posX + entityTexture.getWidth() * 9 / 10,
			topA = posY + entityTexture.getHeight() / 10,
			bottomA = posY + entityTexture.getHeight() * 9 / 10;

		int leftB = other.posX,
			rightB = other.posX + other.entityTexture.getWidth(),
			topB = other.posY,
			bottomB = other.posY + other.entityTexture.getHeight();

		if(leftA >= rightB || rightA <= leftB || bottomA <= topB || topA >= bottomB) return false;
		return true;
	}
	void move(vector<SDL_Rect>& collider, Entity& player) {
		//for bot movement (still in development)
		if(autonomous) {
			if(followPlayer) {
				//bot follow player
			}
			else {
				posX += (flip == SDL_FLIP_NONE ? entityVel : -entityVel);
				if((flip == SDL_FLIP_NONE && posX > SCREEN_WIDTH) || (flip == SDL_FLIP_HORIZONTAL && posX < -entityTexture.getWidth())) {
					setVeclocity();
					setPosition();
				}
				if(collided(player)) {
					EXPLODED = DEAD = true;
				}
			}
		}
		//for player movement
		else {
			posX += velX;
			if(!onGround) {
				posY += velY;
				velY += ACCELERATOR;
			}
			//check player collide with any platform
			bool collideBottom = false;
			for(auto& other : collider) {
				int leftA = posX,
					rightA = posX + entityTexture.getWidth(),
					topA = posY,
					bottomA = posY + entityTexture.getHeight();

				int leftB = other.x,
					rightB = other.x + other.w,
					topB = other.y,
					bottomB = other.y + other.h;
				//vertical collide
				if(leftA < rightB && rightA > leftB) {
					if(bottomA >= topB && topA + entityTexture.getHeight() * 2 / 3 < topB) {
						bottomA = topB;
						topA = posY = topB - entityTexture.getHeight();
						collideBottom = true;
					}
					else if(topA <= bottomB && bottomA - entityTexture.getHeight() * 2 / 3 > bottomB) {
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
				if(velX == 0) currentSprite = STAY_STILL;
				else if(velX > 0) currentSprite = MOVE_RIGHT;
				else currentSprite = MOVE_LEFT;
			}
		}
		//change to next frame of animation
		currenSubSprite = (currenSubSprite + 1) % subSprite[currentSprite];
	}
};

//shared item
const int BUTTON_WIDTH = 100;
const int BUTTON_HEIGHT = 70;
TTF_Font* font;
Mix_Chunk* touch_button = nullptr;
Mix_Chunk* click_button = nullptr;
Button back;
bool touch_back, lock_back_button_sound;

//menu
Texture menu_background, menu_title;
Button start, instruction, high_score;
Mix_Music* menu_music = nullptr;
bool touch_1, touch_2, touch_3, lock_menu_button_sound;

//instruction
Texture instruction_background;

//play
string player_name;
Uint32 survival_time;

//high score
Texture high_score_background, high_score_title;
vector<pair<string, Uint32>> score_data;

void init() {
	//control game path
	MENU = true;
	PLAY = INSTRUCTION = HIGH_SCORE = EXPLODED = SHADOW_CAUGHT = PAUSE = DEAD = false;

	//initialize SDL
	SDL_Init(SDL_INIT_EVERYTHING);
	//initialize image
	IMG_Init(IMG_INIT_PNG);
	//initialize font
	TTF_Init();
	//initialize screen and renderer
	window = SDL_CreateWindow("Dark forest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//initialize audio
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	//shared event
	touch_back = lock_back_button_sound = false;

	//menu event
	touch_1 = touch_2 = touch_3 = lock_menu_button_sound = false;

	//high score
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
void loadMedia() {
	//choose font
	font = TTF_OpenFont("font/Cabanyal-Z.ttf", 50);
	//touch button sound
	touch_button = Mix_LoadWAV("sound/touch_button.wav");
	Mix_VolumeChunk(touch_button, 100);
	//click button sound
	click_button = Mix_LoadWAV("sound/click_button.wav");
	Mix_VolumeChunk(click_button, 100);
	//back button
	back.textureFromText("Back", font, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) back.setSprite(i, 0, 0, back.buttonWidth(), back.buttonHeight());
	back.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	back.setPosition(25, 25);

	//set up menu region
	//menu background
	menu_background.loadFromFile("texture/menu_background.png");
	menu_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	//game title
	menu_title.loadFromRenderedText("Dark forest", font, {169,50,38});
	menu_title.setSize(500, 175);
	//start button
	start.textureFromText("Start", font, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) start.setSprite(i, 0, 0, start.buttonWidth(), start.buttonHeight());
	start.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	start.setPosition((SCREEN_WIDTH - start.buttonWidth()) / 2, 250);
	//instruction button
	instruction.textureFromText("Instruction", font, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) instruction.setSprite(i, 0, 0, instruction.buttonWidth(), instruction.buttonHeight());
	instruction.setButtonSize(BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	instruction.setPosition((SCREEN_WIDTH - instruction.buttonWidth()) / 2, 370);
	//high score button
	high_score.textureFromText("High score", font, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) high_score.setSprite(i, 0, 0, high_score.buttonWidth(), high_score.buttonHeight());
	high_score.setButtonSize(BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	high_score.setPosition((SCREEN_WIDTH - high_score.buttonWidth()) / 2, 490);
	//menu music
	menu_music = Mix_LoadMUS("sound/menu_music.wav");
	Mix_VolumeMusic(40);

	//set up instruction region
	//instruction background
	instruction_background.loadFromFile("texture/instruction_background.png");
	instruction_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//set up high score region
	//high score background
	high_score_background.loadFromFile("texture/high_score_background.png");
	high_score_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	high_score_title.loadFromRenderedText("Scoreboard", font, {255,0,255});
	high_score_title.setSize(250, 100);
}
void close() {
	//close shared item
	TTF_CloseFont(font);
	font = nullptr;
	Mix_FreeChunk(touch_button);
	touch_button = nullptr;
	Mix_FreeChunk(click_button);
	click_button = nullptr;
	back.free();

	//close menu region
	menu_background.free();
	menu_title.free();
	start.free();
	instruction.free();
	high_score.free();
	Mix_FreeMusic(menu_music);
	menu_music = nullptr;

	//close instruction region
	instruction_background.free();

	//close high score region
	score_data.resize(0);
	high_score_background.free();
	high_score_title.free();

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

int main(int argc, char** argv) {
	init();
	loadMedia();
	bool quit = false;
	SDL_Event e;
	stringstream text;
	while(!quit) {
		//menu region
		if(MENU) {
			if(Mix_PlayingMusic() == 0) Mix_PlayMusic(menu_music, 1);
			while(SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) quit = true;
				start.handleEvent(&e);
				instruction.handleEvent(&e);
				high_score.handleEvent(&e);
			}
			switch(start.getSprite()) {
				case MOUSE_OUT:
					touch_1 = false;
					start.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_1 = true;
					start.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					break;
			}
			switch(instruction.getSprite()) {
				case MOUSE_OUT:
					touch_2 = false;
					instruction.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_2 = true;
					instruction.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					MENU = false;
					INSTRUCTION = true;
					back.setSpriteState(MOUSE_OUT);
					Mix_PlayChannel(-1, click_button, 0);
					break;
			}
			switch(high_score.getSprite()) {
				case MOUSE_OUT:
					touch_3 = false;
					high_score.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_3 = true;
					high_score.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					MENU = false;
					HIGH_SCORE = true;
					back.setSpriteState(MOUSE_OUT);
					Mix_PlayChannel(-1, click_button, 0);
					break;
			}
			if(MENU) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				menu_background.render(0, 0);
				menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) / 2, 25);
				start.render();
				instruction.render();
				high_score.render();
				SDL_RenderPresent(renderer);
				if(touch_1 || touch_2 || touch_3) {
					if(!lock_menu_button_sound) {
						lock_menu_button_sound = true;
						Mix_PlayChannel(-1, touch_button, 0);
					}
				}
				else lock_menu_button_sound = false;
			}
		}
		//play region
		if(PLAY) {}
		//instruction region
		if(INSTRUCTION) {
			while(SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) quit = true;
				back.handleEvent(&e);
			}
			switch(back.getSprite()) {
				case MOUSE_OUT:
					touch_back = false;
					back.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_back = true;
					back.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					MENU = true;
					INSTRUCTION = false;
					instruction.setSpriteState(MOUSE_OUT);
					Mix_PlayChannel(-1, click_button, 0);
					break;
			}
			if(INSTRUCTION) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				instruction_background.render(0, 0);
				back.render();
				SDL_RenderPresent(renderer);
				if(touch_back) {
					if(!lock_back_button_sound) {
						lock_back_button_sound = true;
						Mix_PlayChannel(-1, touch_button, 0);
					}
				}
				else lock_back_button_sound = false;
			}
		}
		//high score region
		if(HIGH_SCORE) {
			while(SDL_PollEvent(&e) != 0) {
				if(e.type == SDL_QUIT) quit = true;
				back.handleEvent(&e);
			}
			switch(back.getSprite()) {
				case MOUSE_OUT:
					touch_back = false;
					back.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					touch_back = true;
					back.setButtonColor(255, 0, 0);
					break;
				case MOUSE_DOWN:
					MENU = true;
					HIGH_SCORE = false;
					high_score.setSpriteState(MOUSE_OUT);
					Mix_PlayChannel(-1, click_button, 0);
					break;
			}
			if(HIGH_SCORE) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);
				high_score_background.render(0, 0);
				high_score_title.render((SCREEN_WIDTH - high_score_title.getWidth()) / 2, 75);
				back.render();
				for(int i = 0; i < score_data.size(); i++) {
					stringstream text;
					text.str("");
					text << i + 1 << "." << (score_data[i].second != 0 ? score_data[i].first : "none") << string(10, ' ') << score_data[i].second;
					SDL_Color text_color;
					switch(i) {
						case 0:
							text_color = {255,215,0};
							break;
						case 1:
							text_color = {192,192,192};
							break;
						default:
							text_color = {205, 127, 50};
							break;
					}
					Texture data_information;
					data_information.loadFromRenderedText(text.str(), font, text_color);
					data_information.setSize(20 * text.str().length(), 75);
					data_information.render((SCREEN_WIDTH - data_information.getWidth()) / 2, 200 + i * 75);
					data_information.free();
				}
				SDL_RenderPresent(renderer);
				if(touch_back) {
					if(!lock_back_button_sound) {
						lock_back_button_sound = true;
						Mix_PlayChannel(-1, touch_button, 0);
					}
				}
				else lock_back_button_sound = false;
			}
		}
	}
	close();
	return 0;
}