#pragma once
#ifndef MEDIA
#define MEDIA
#include"basicVariable.h"
#include"advancedVariable.h"
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
	Mix_VolumeChunk(touch_button, MAX_VOLUME);
	//click button sound
	click_button = Mix_LoadWAV("sound/click_button.wav");
	Mix_VolumeChunk(click_button, MAX_VOLUME);
	//back button
	back.textureFromText("Back", button_font, {255,255,255});
	back.setButtonSize(BUTTON_WIDTH * 4 / 5, BUTTON_HEIGHT * 4 / 5);
	back.setPosition(25, 25);
	//menu background
	menu_background.loadFromFile("texture/menu_background.png");
	menu_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	//set up menu region
	//game title
	menu_title.loadFromRenderedText("Dark forest", deco_font, {138,3,3});
	menu_title.setSize(550, 175);
	//start button
	play.textureFromText("Play", button_font, {255,255,255});
	play.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	play.setPosition((SCREEN_WIDTH - play.buttonWidth()) >> 1, 250);
	//instruction button
	instruction.textureFromText("Instruction", button_font, {255,255,255});
	instruction.setButtonSize(BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	instruction.setPosition((SCREEN_WIDTH - instruction.buttonWidth()) >> 1, 370);
	//high score button
	high_score.textureFromText("High score", button_font, {255,255,255});
	high_score.setButtonSize(BUTTON_WIDTH * 2, BUTTON_HEIGHT);
	high_score.setPosition((SCREEN_WIDTH - high_score.buttonWidth()) >> 1, 490);
	//menu music
	menu_music = Mix_LoadMUS("sound/menu_music.wav");
	Mix_VolumeMusic(MAX_VOLUME >> 2);

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
	player.setPosition((SCREEN_WIDTH - player.entityWidth()) >> 1, (SCREEN_HEIGHT - player.entityHeight()) >> 1);
	//player velocity
	player.setVeclocity(4);
	//player move sound
	player.loadMoveSound("sound/player_move.wav", MAX_VOLUME);
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
		e.setAmountOfSprite(STAY_STILL, 1);
		for(int i = 0; i < e.getNumberSubSprite(STAY_STILL); i++) e.setEntitySprite(STAY_STILL, i, 0, 0, 100, 120);
		e.setEntitySize(35, 42);
		e.setVeclocity();
		e.setPosition();
	}
	identification = 0;
	skull_warning.loadFromFile("texture/skull_warning.png");
	skull_warning.setSize(50, 50);
	exploded_texture.loadFromFile("texture/exploded.png");
	exploded_texture.setSize(42, 42);
	exploded_sound = Mix_LoadWAV("sound/exploded.wav");
	Mix_VolumeChunk(exploded_sound, MAX_VOLUME);
	//grim reaper
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
		e.loadDeadSound("sound/reaper_death.wav", MAX_VOLUME >> 2);
	}
	identification = 0;
	reaper_warning.loadFromFile("texture/reaper_warning.png");
	reaper_warning.setSize(45, 60);
	shadow_caught_texture.loadFromFile("texture/shadow_caught.png", 100, 100, 100);
	shadow_caught_texture.setSize(60, 60);
	shadow_caught_sound = Mix_LoadWAV("sound/shadow_caught.wav");
	Mix_VolumeChunk(shadow_caught_sound, MAX_VOLUME);
	//pause game button
	pause_game.textureFromText("Pause", button_font, {255,255,255});
	pause_game.setButtonSize(BUTTON_WIDTH * 4 / 5, BUTTON_HEIGHT * 4 / 5);
	pause_game.setPosition(15, 10);
	//support board
	support_board.loadFromFile("texture/support_board.png");
	support_board.setSize(150, 200);
	//resume game button
	resume_game.textureFromText("Resume", button_font, {255,255,255});
	resume_game.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	resume_game.setPosition((SCREEN_WIDTH - resume_game.buttonWidth()) >> 1, 255);
	//exit game button
	exit_game.textureFromText("Exit", button_font, {255,255,255});
	exit_game.setButtonSize(BUTTON_WIDTH * 4 / 5, BUTTON_HEIGHT);
	exit_game.setPosition((SCREEN_WIDTH - exit_game.buttonWidth()) >> 1, 345);
	//dead background
	dead_background.loadFromFile("texture/dead_background.png");
	dead_background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	dead_background.setBlendMode(SDL_BLENDMODE_BLEND);

	//set up instruction region

	//set up high score region
	//reset button
	resetScore.textureFromText("Reset", button_font, {255,255,255});
	resetScore.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	resetScore.setPosition((SCREEN_WIDTH - 25 - resetScore.buttonWidth()), 25);
}
#endif // !MEDIA
