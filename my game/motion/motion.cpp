//https://TOKEN@github.com/USERNAME/REPO.git

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<string>
#include<sstream>
#include<vector>
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int ACCELERATOR = 1;

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
	//void setBlendMode(SDL_BlendMode blending) {
	//	SDL_SetTextureBlendMode(currentTexture, blending);
	//}
	//void setAlpha(Uint8 alpha) {
	//	SDL_SetTextureAlphaMod(currentTexture, alpha);
	//}
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

enum entityMovement {
	STAY_STILL,
	MOVE_LEFT,
	MOVE_RIGHT,
	JUMP,
	TOTAL_MOVEMENT
};
class Entity {
	int posX, posY, velX, velY, entityVel, currentSprite, currenSubSprite;
	bool autonomous, collide, inScreen, onGround;

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
		autonomous = collide = inScreen = onGround = false;
		flip = SDL_FLIP_NONE;
		currenSubSprite = 0;
		subSprite.resize(TOTAL_MOVEMENT);
		entitySpriteClip.resize(TOTAL_MOVEMENT);
	}
	void setAuto() {
		autonomous = true;
	}
	void setCollide() {
		collide = true;
	}
	void setInScreen() {
		inScreen = true;
	}
	void setVeclocity(int v) {
		entityVel = v;
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
	}
	void setPosition(int x, int y) {
		posX = x;
		posY = y;
	}
	void renderEntity() {
		entityTexture.render(posX, posY, &entitySpriteClip[currentSprite][currenSubSprite], 0.0, nullptr, flip);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_Rect playerSquare = {posX,posY,entityTexture.getWidth(),entityTexture.getHeight()};
		//SDL_RenderDrawRect(renderer, &playerSquare);
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
		if(currentSprite != oldSprite) currenSubSprite = -1;
	}
	void move(vector<Entity>& collider) {
		posX += velX;
		if(!onGround) {
			posY += velY;
			velY += ACCELERATOR;
		}

		if(collide) {
			bool collideBottom = false;
			for(auto& other : collider) {
				int leftA = posX,
					rightA = posX + entityTexture.getWidth(),
					topA = posY,
					bottomA = posY + entityTexture.getHeight();

				int leftB = other.posX,
					rightB = other.posX + other.entityTexture.getWidth(),
					topB = other.posY,
					bottomB = other.posY + other.entityTexture.getHeight();

				if(leftA < rightB && rightA > leftB) {
					if(bottomA >= topB && topA + (entityTexture.getHeight()) * 2 / 3 < topB) {
						bottomA = topB;
						topA = posY = topB - entityTexture.getHeight();
						collideBottom = true;
					}
					else if(topA <= bottomB && bottomA > bottomB) {
						topA = posY = bottomB;
						bottomA = bottomB + entityTexture.getHeight();
						velY = 1;
					}
				}

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
		}
		if(inScreen) {
			if(posX < 0) {
				posX = 0;
			}
			else if(posX + entityTexture.getWidth() > SCREEN_WIDTH) {
				posX = SCREEN_WIDTH - entityTexture.getWidth();
			}
			if(posY < 0) {
				posY = 0;
			}
			else if(posY + entityTexture.getHeight() > SCREEN_HEIGHT) {
				posY = SCREEN_HEIGHT - entityTexture.getHeight();
				onGround = true;
			}
		}
		if(onGround) {
			velY = 0;
			if(velX == 0) currentSprite = STAY_STILL;
			else if(velX > 0) currentSprite = MOVE_RIGHT;
			else currentSprite = MOVE_LEFT;
		}
		currenSubSprite = (currenSubSprite + 1) % subSprite[currentSprite];
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
};

Texture background;
Entity player;
vector<Entity> platform(3);

void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark jump", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);
}
void loadMedia() {
	background.loadFromFile("texture/background.png");
	background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	player.setInScreen();
	player.setCollide();

	player.loadEntityTexture("texture/player.png", 0, 0, 0);

	player.setAmountOfSprite(STAY_STILL, 7);
	for(int i = 0; i < player.getNumberSubSprite(STAY_STILL); i++) player.setEntitySprite(STAY_STILL, i, 96 * i + 32, 84 + 40, 32, 44);
	player.setAmountOfSprite(MOVE_RIGHT, 8);
	for(int i = 0; i < player.getNumberSubSprite(MOVE_RIGHT); i++) player.setEntitySprite(MOVE_RIGHT, i, 96 * i + 32, 252 + 40, 32, 44);
	player.setAmountOfSprite(MOVE_LEFT, 8);
	for(int i = 0; i < player.getNumberSubSprite(MOVE_LEFT); i++) player.setEntitySprite(MOVE_LEFT, i, 96 * i + 32, 252 + 40, 32, 44);
	player.setAmountOfSprite(JUMP, 1);
	player.setEntitySprite(JUMP, 0, 96 + 32, 336 + 40, 32, 44);

	player.setEntitySize(32, 44);
	//player.setPosition((SCREEN_WIDTH - player.entityWidth()) / 2, (SCREEN_HEIGHT - player.entityHeight()) / 2);
	//player.setPosition(0, (SCREEN_HEIGHT - player.entityHeight()) / 2);
	player.setPosition((SCREEN_WIDTH - player.entityWidth()) / 2, (SCREEN_WIDTH - player.entityHeight()) / 2);

	player.setVeclocity(5);

	platform[0].loadEntityTexture("texture/longDirt.png");
	platform[0].setAmountOfSprite(STAY_STILL, 1);
	platform[0].setEntitySprite(STAY_STILL, 0, 0, 0, platform[0].entityWidth(), platform[0].entityHeight());
	platform[0].setEntitySize(300, 100);
	platform[0].setPosition(0, SCREEN_HEIGHT - platform[0].entityHeight() + 35);

	platform[1].loadEntityTexture("texture/longDirt.png");
	platform[1].setAmountOfSprite(STAY_STILL, 1);
	platform[1].setEntitySprite(STAY_STILL, 0, 0, 0, platform[1].entityWidth(), platform[1].entityHeight());
	platform[1].setEntitySize(300, 100);
	platform[1].setPosition(250, SCREEN_HEIGHT - platform[1].entityHeight() + 50);

	platform[2].loadEntityTexture("texture/longDirt.png");
	platform[2].setAmountOfSprite(STAY_STILL, 1);
	platform[2].setEntitySprite(STAY_STILL, 0, 0, 0, platform[2].entityWidth(), platform[2].entityHeight());
	platform[2].setEntitySize(300, 100);
	platform[2].setPosition(500, SCREEN_HEIGHT - platform[0].entityHeight() + 65);
}
void close() {
	background.free();
	player.free();
	for(auto& P : platform) P.free();
	platform.resize(0);

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
		background.render(0, 0);
		for(auto& P : platform) P.renderEntity();

		player.move(platform);
		player.renderEntity();

		SDL_RenderPresent(renderer);
		SDL_Delay(20);
	}
	close();
	return 0;
}