//https://TOKEN@github.com/USERNAME/REPO.git

#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<string>
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* titleFont = nullptr;
TTF_Font* buttonFont = nullptr;

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
	void loadFromFile(string path) {
		free();
		SDL_Surface* loadedImage = IMG_Load(path.c_str());
		SDL_SetColorKey(loadedImage, SDL_TRUE, SDL_MapRGB(loadedImage->format, 1, 1, 1));
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
	void setTextureSize(int w, int h) {
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
	SDL_Point position;
	ButtonSprite currentSprite;
	SDL_Rect buttonSpriteClip[MOUSE_TOTAL];
	Texture buttonTexture;
public:
	Button() {
		position.x = position.y = 0;
		currentSprite = MOUSE_OUT;
		for(int i = 0; i < MOUSE_TOTAL; i++) buttonSpriteClip[i] = {0,0,buttonTexture.getWidth(),buttonTexture.getHeight()};
	}
	void setPosition(int x, int y) {
		position.x = x;
		position.y = y;
	}
	void handleEvent(SDL_Event* e) {
		if(e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			bool inside = true;
			if(x<position.x || x>position.x + buttonTexture.getWidth() || y<position.y || y>position.y + buttonTexture.getHeight()) inside = false;
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
	void textureFromFile(string path) {
		buttonTexture.loadFromFile(path);
	}
	void textureFromText(string text, TTF_Font* font, SDL_Color color) {
		buttonTexture.loadFromRenderedText(text, font, color);
	}
	void setButtonSize(int w, int h) {
		buttonTexture.setTextureSize(w, h);
	}
	void setButtonColor(Uint8 red, Uint8 green, Uint8 blue) {
		buttonTexture.setColor(red, green, blue);
	}
	void setSprite(int sprite, int sprite_x, int sprite_y, int sprite_width, int sprite_height) {
		buttonSpriteClip[sprite] = {sprite_x,sprite_y,sprite_width,sprite_height};
	}
	void render() {
		buttonTexture.render(position.x, position.y, &buttonSpriteClip[currentSprite]);
	}
	int getSprite() {
		return currentSprite;
	}
	int textureWidth() {
		return buttonTexture.getWidth();
	}
	int textureHeight() {
		return buttonTexture.getHeight();
	}
	void free() {
		buttonTexture.free();
	}
};

Texture background, title;
Button start, instruction, high_score;

void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark form", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}
void loadMedia() {
	titleFont = TTF_OpenFont("font/ZombieGirlfriend.ttf", 50);
	buttonFont = TTF_OpenFont("font/Cabanyal-Z.ttf", 50);

	background.loadFromFile("texture/background.png");
	background.setTextureSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	title.loadFromRenderedText("DARK FORM", titleFont, {200,10,10});
	title.setTextureSize(300, 150);

	start.textureFromText("Start", buttonFont, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) start.setSprite(i, 0, 0, start.textureWidth(), start.textureHeight());
	start.setButtonSize(150, 75);
	start.setPosition((SCREEN_WIDTH - 150) / 2, SCREEN_HEIGHT / 4 + 50);

	instruction.textureFromText("Instruction", buttonFont, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) instruction.setSprite(i, 0, 0, instruction.textureWidth(), instruction.textureHeight());
	instruction.setButtonSize(150, 75);
	instruction.setPosition((SCREEN_WIDTH - 150) / 2, SCREEN_HEIGHT / 4 + 120 + 50);

	high_score.textureFromText("High score", buttonFont, {255,255,255});
	for(int i = 0; i < MOUSE_TOTAL; i++) high_score.setSprite(i, 0, 0, high_score.textureWidth(), high_score.textureHeight());
	high_score.setButtonSize(150, 75);
	high_score.setPosition((SCREEN_WIDTH - 150) / 2, SCREEN_HEIGHT / 4 + 240 + 50);
}
void close() {
	start.free();
	instruction.free();
	high_score.free();

	TTF_CloseFont(titleFont);
	TTF_CloseFont(buttonFont);
	titleFont = nullptr;
	buttonFont = nullptr;

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = nullptr;
	window = nullptr;

	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char** argv) {
	init();
	loadMedia();
	bool quit = false;
	SDL_Event e;
	while(!quit) {
		while(SDL_PollEvent(&e) != 0) {
			if(e.type == SDL_QUIT) quit = true;
			start.handleEvent(&e);
			instruction.handleEvent(&e);
			high_score.handleEvent(&e);

			switch(start.getSprite()) {
				case MOUSE_OUT:
					start.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					start.setButtonColor(255, 0, 0);
					break;
			}

			switch(instruction.getSprite()) {
				case MOUSE_OUT:
					instruction.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					instruction.setButtonColor(255, 0, 0);
					break;
			}

			switch(high_score.getSprite()) {
				case MOUSE_OUT:
					high_score.setButtonColor(255, 255, 255);
					break;
				case MOUSE_IN:
					high_score.setButtonColor(255, 0, 0);
					break;
			}

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderClear(renderer);
			background.render(0, 0);
			title.render((SCREEN_WIDTH - title.getWidth()) / 2, 30);
			start.render();
			instruction.render();
			high_score.render();
			SDL_RenderPresent(renderer);
		}
	}
	close();
	return 0;
}