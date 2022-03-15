//#pragma once
#ifndef BUTTON
#define BUTTON
#include"texture.h"
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
			if(x < posX || x > posX + buttonTexture.textureWidth() || y < posY || y > posY + buttonTexture.textureHeight()) currentSprite = MOUSE_OUT;
			else switch(e->type) {
				case SDL_MOUSEMOTION: currentSprite = MOUSE_IN; return;
				case SDL_MOUSEBUTTONDOWN: currentSprite = MOUSE_DOWN; return;
			}
		}
	}
	void textureFromFile(std::string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		buttonTexture.loadFromFile(path, r, g, b);
	}
	void textureFromText(std::string text, TTF_Font* font, SDL_Color color) {
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
		return buttonTexture.textureWidth();
	}
	int buttonHeight() {
		return buttonTexture.textureHeight();
	}
	void reset() {
		currentSprite = MOUSE_OUT;
	}
	void free() {
		buttonTexture.free();
	}
};

//shared items
Button back;

//menu
Button play, instruction, high_score;

//game play
Button pause_game, resume_game, exit_game;

//high score
Button resetScore;
#endif // !BUTTON
