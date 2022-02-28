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
TTF_Font* font1 = nullptr;
TTF_Font* font2 = nullptr;

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
		SDL_SetColorKey(loadedImage, SDL_TRUE, SDL_MapRGB(loadedImage->format, 255, 255, 255));
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
	~Texture() { free(); }
};

const int BUTTON_WIDTH = 150;
const int BUTTON_HEIGHT = 75;
enum ButtonSprite {
	MOUSE_IN,
	MOUSE_OUT,
	MOUSE_DOWN,
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
		for(int i = 0; i < MOUSE_TOTAL; i++) buttonSpriteClip[i] = {0,0,BUTTON_WIDTH,BUTTON_HEIGHT};
	}
	void setPosition(int x, int y) {
		position.x = x;
		position.y = y;
	}
	void handleEvent(SDL_Event* e) {
		if(e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN) {
			int x, y;
			SDL_GetMouseState(&x, &y);
			bool inside = true;
			if(x<position.x || x>position.x + BUTTON_WIDTH || y<position.y || y>position.y + BUTTON_HEIGHT) inside = false;
			if(!inside) currentSprite = MOUSE_OUT;
			else {
				switch(e->type) {
					case SDL_MOUSEMOTION:
						currentSprite = MOUSE_IN;
						break;
					case SDL_MOUSEBUTTONDOWN:
						currentSprite = MOUSE_DOWN;
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
	void setSprite(ButtonSprite sprite, int sprite_x, int sprite_y, int sprite_width, int sprite_height) {
		buttonSpriteClip[sprite] = {sprite_x,sprite_y,sprite_width,sprite_height};
	}
	void render() {
		buttonTexture.render(position.x, position.y, &buttonSpriteClip[currentSprite]);
	}
	ButtonSprite getSprite() {
		return currentSprite;
	}
	void free() {
		buttonTexture.free();
	}
};

void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark form", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}
void loadMedia() {
}
void close() {
	TTF_CloseFont(font1);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	font1 = nullptr;
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
		}
	}
	close();
	return 0;
}