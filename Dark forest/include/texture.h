#pragma once
#ifndef TEXTURsE
#define TEXTURE
#include"basicVariable.h"
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
	void loadFromFile(std::string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		free();
		SDL_Surface* loadedImage = IMG_Load(path.c_str());
		SDL_SetColorKey(loadedImage, SDL_TRUE, SDL_MapRGB(loadedImage->format, r, g, b));
		currentTexture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		width = loadedImage->w;
		height = loadedImage->h;
		SDL_FreeSurface(loadedImage);
		loadedImage = nullptr;
	}
	void loadFromRenderedText(std::string textureText, TTF_Font* font, SDL_Color textColor) {
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

//shared items
Texture transition_screen;

//menu
Texture menu_background, menu_title;

//game play
Texture game_background, game_map, skull_warning, exploded_texture, reaper_warning, reaper_death_texture, shadow_caught_texture, support_board, dead_background;

//instruction
Texture instruction_background;

//high score
Texture high_score_background, high_score_title;
#endif // !TEXTURE
