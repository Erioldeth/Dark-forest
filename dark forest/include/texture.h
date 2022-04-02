#ifndef TEXTURE_H
#define TEXTURE_H
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
	void loadTextureFromFile(std::string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		free();
		SDL_Surface* loadedImage = IMG_Load(path.c_str());
		SDL_SetColorKey(loadedImage, SDL_TRUE, SDL_MapRGB(loadedImage->format, r, g, b));
		currentTexture = SDL_CreateTextureFromSurface(renderer, loadedImage);
		SDL_FreeSurface(loadedImage);
		loadedImage = nullptr;
	}
	void loadTextureFromText(std::string textureText, TTF_Font* font, SDL_Color text_color = {255,255,255}) {
		free();
		SDL_Surface* loadedText = TTF_RenderText_Solid(font, textureText.c_str(), text_color);
		currentTexture = SDL_CreateTextureFromSurface(renderer, loadedText);
		width = loadedText->w;
		height = loadedText->h;
		SDL_FreeSurface(loadedText);
		loadedText = nullptr;
	}
	void setTextureSize(int w, int h) {
		width = w;
		height = h;
	}
	void setTextureColor(Uint8 red, Uint8 green, Uint8 blue) {
		SDL_SetTextureColorMod(currentTexture, red, green, blue);
	}
	void setTextureBlendMode(SDL_BlendMode blending) {
		SDL_SetTextureBlendMode(currentTexture, blending);
	}
	void setTextureAlpha(Uint8 alpha) {
		SDL_SetTextureAlphaMod(currentTexture, alpha);
	}
	void renderTexture(int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE) {
		SDL_Rect renderQuad{x,y,width,height};
		SDL_RenderCopyEx(renderer, currentTexture, clip, &renderQuad, angle, center, flip);
	}
	int textureWidth() { return width; }
	int textureHeight() { return height; }
};

//shared items
Texture transition_screen;
Texture menu_background;

//menu
Texture game_title;

//game play
Texture game_background, game_map;
Texture skull_warning, exploded_texture;
Texture reaper_warning, shadow_caught_texture;
std::deque<std::tuple<Texture, SDL_Point, int>> reaper_death;
Texture support_board, support_frame, dead_background;

//setting
std::vector<Texture> volume_name(2), volume_block(2), picker_name(2), picker_frame(2);
Texture model_frame, player_model;
#endif // !TEXTURE_H
