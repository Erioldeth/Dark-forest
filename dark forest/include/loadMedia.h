#ifndef MEDIA_H
#define MEDIA_H
#include"advancedVariable.h"
void loadMedia() {
	//share items
	menu_background.loadTextureFromFile("texture/menu_background.png");
	menu_background.setTextureSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	transition_screen.loadTextureFromFile("texture/transition_screen.png");
	transition_screen.setTextureSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	transition_screen.setTextureBlendMode(SDL_BLENDMODE_BLEND);

	button_font = TTF_OpenFont("font/Cabanyal-Z.ttf", 60);
	deco_font = TTF_OpenFont("font/ZombieGirlfriend.ttf", 60);

	touch_button = Mix_LoadWAV("sound/touch_button.wav");
	Mix_VolumeChunk(touch_button, 10 * volume_ratio[1]);

	click_button = Mix_LoadWAV("sound/click_button.wav");
	Mix_VolumeChunk(click_button, 10 * volume_ratio[1]);

	back.textureFromText("Back", button_font, {255,255,255});
	back.setButtonSize((BUTTON_WIDTH << 2) / 5, (BUTTON_HEIGHT << 2) / 5);
	back.setPosition(25, 25);

	reset.textureFromText("Reset", button_font);
	reset.setButtonSize(BUTTON_WIDTH, (BUTTON_HEIGHT << 2) / 5);
	reset.setPosition((SCREEN_WIDTH - 25 - reset.buttonWidth()), 25);



	//set up menu region
	game_title.loadTextureFromText("Dark forest", deco_font, {138,3,3});
	game_title.setTextureSize(550, 175);

	play.textureFromText("Play", button_font);
	play.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	play.setPosition((SCREEN_WIDTH - play.buttonWidth()) >> 1, 250);

	setting.textureFromText("Setting", button_font);
	setting.setButtonSize((BUTTON_WIDTH * 3) >> 1, BUTTON_HEIGHT);
	setting.setPosition((SCREEN_WIDTH - setting.buttonWidth()) >> 1, 370);

	high_score.textureFromText("High score", button_font);
	high_score.setButtonSize(BUTTON_WIDTH << 1, BUTTON_HEIGHT);
	high_score.setPosition((SCREEN_WIDTH - high_score.buttonWidth()) >> 1, 490);



	//set up game play
	easy_mode.textureFromText("Easy", button_font);
	easy_mode.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	easy_mode.setPosition((SCREEN_WIDTH - easy_mode.buttonWidth()) >> 1, 165);
	normal_mode.textureFromText("Normal", button_font);
	normal_mode.setButtonSize((BUTTON_WIDTH * 3) >> 1, BUTTON_HEIGHT);
	normal_mode.setPosition((SCREEN_WIDTH - normal_mode.buttonWidth()) >> 1, 285);
	hard_mode.textureFromText("Hard", button_font);
	hard_mode.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	hard_mode.setPosition((SCREEN_WIDTH - hard_mode.buttonWidth()) >> 1, 405);

	game_map.loadTextureFromFile("texture/game_map.png");
	game_map.setTextureSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	game_background.loadTextureFromFile("texture/game_background.png");
	game_background.setTextureSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	player.loadEntityTexture("texture/player.png");
	player.setEntityColor(color_code[0][0], color_code[0][1], color_code[0][2]);
	player.setParticleColor(color_code[1][0], color_code[1][1], color_code[1][2]);
	player.setNumberOfAnimation(STAY_STILL, 35);
	for(int i = 0; i < player.getNumberOfAnimation(STAY_STILL); i++) player.setEntityAnimation(STAY_STILL, i, 96 * (i / 5) + 32, 84 + 42, 32, 42);
	player.setNumberOfAnimation(MOVE_RIGHT, 40);
	for(int i = 0; i < player.getNumberOfAnimation(MOVE_RIGHT); i++) player.setEntityAnimation(MOVE_RIGHT, i, 96 * (i / 5) + 32, 252 + 42, 32, 42);
	player.setNumberOfAnimation(MOVE_LEFT, 40);
	for(int i = 0; i < player.getNumberOfAnimation(MOVE_LEFT); i++) player.setEntityAnimation(MOVE_LEFT, i, 96 * (i / 5) + 32, 252 + 42, 32, 42);
	player.setNumberOfAnimation(JUMP, 1);
	player.setEntityAnimation(JUMP, 0, 96 + 32, 336 + 40, 32, 42);
	player.setNumberOfAnimation(SIT, 30);
	for(int i = 0; i < player.getNumberOfAnimation(SIT); i++) player.setEntityAnimation(SIT, i, 96 * (i / 5) + 32, 756 + 56, 32, 28);
	player.setEntitySize(32, 42);
	player.setPosition((SCREEN_WIDTH - player.entityWidth()) >> 1, (SCREEN_HEIGHT - player.entityHeight()) >> 1);
	player.setVeclocity(4);
	player.loadMoveSound("sound/player_move.wav", 10 * volume_ratio[1]);

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

	for(auto& e : skull) {
		e.setID(identification++);
		e.setAuto();
		e.loadEntityTexture("texture/skull.png");
		e.setNumberOfAnimation(STAY_STILL, 1);
		for(int i = 0; i < e.getNumberOfAnimation(STAY_STILL); i++) e.setEntityAnimation(STAY_STILL, i, 0, 0, 100, 120);
		e.setEntitySize(30, 36);
		e.setVeclocity();
		e.setPosition();
	}
	identification = 0;
	skull_warning.loadTextureFromFile("texture/skull_warning.png");
	skull_warning.setTextureSize(40, 40);
	exploded_texture.loadTextureFromFile("texture/exploded.png", 0, 255);
	exploded_texture.setTextureSize(60, 60);
	exploded_sound = Mix_LoadWAV("sound/exploded.wav");
	Mix_VolumeChunk(exploded_sound, 10 * volume_ratio[1]);

	for(auto& e : reaper) {
		e.setID(identification++);
		e.setAuto();
		e.setFollowPlayer();
		e.loadEntityTexture("texture/reaper.png");
		e.setNumberOfAnimation(STAY_STILL, 90);
		for(int i = 0; i < e.getNumberOfAnimation(STAY_STILL); i++) e.setEntityAnimation(STAY_STILL, i, 48 * (i / 10), 0, 48, 48);
		e.setEntitySize(48, 48);
		e.setVeclocity();
		e.setPosition();
		e.loadDeadSound("sound/reaper_death.wav", 3 * volume_ratio[1]);
	}
	reaper_warning.loadTextureFromFile("texture/reaper_warning.png");
	reaper_warning.setTextureSize(30, 40);
	shadow_caught_texture.loadTextureFromFile("texture/shadow_caught.png", 255);
	shadow_caught_texture.setTextureSize(60, 60);
	shadow_caught_sound = Mix_LoadWAV("sound/shadow_caught.wav");
	Mix_VolumeChunk(shadow_caught_sound, 10 * volume_ratio[1]);

	pause_game.textureFromText("Pause", button_font);
	pause_game.setButtonSize((BUTTON_WIDTH << 2) / 5, (BUTTON_HEIGHT << 2) / 5);
	pause_game.setPosition(15, 10);

	support_board.loadTextureFromFile("texture/support_board.png");
	support_board.setTextureSize(150, 200);

	support_frame.loadTextureFromFile("texture/support_frame.png", 255);
	support_frame.setTextureSize(180, 230);
	support_frame.setTextureColor(240, 255, 255);

	resume_game.textureFromText("Resume", button_font);
	resume_game.setButtonSize(BUTTON_WIDTH, BUTTON_HEIGHT);
	resume_game.setPosition((SCREEN_WIDTH - resume_game.buttonWidth()) >> 1, 255);

	exit_game.textureFromText("Exit", button_font);
	exit_game.setButtonSize((BUTTON_WIDTH << 2) / 5, BUTTON_HEIGHT);
	exit_game.setPosition((SCREEN_WIDTH - exit_game.buttonWidth()) >> 1, 345);

	dead_background.loadTextureFromFile("texture/dead_background.png");
	dead_background.setTextureSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	dead_background.setTextureBlendMode(SDL_BLENDMODE_BLEND);



	//set up setting region
	for(int i = 0; i < 2; i++) {
		volume_name[i].loadTextureFromText(!i ? "music" : "sound", deco_font, {240,255,255});
		volume_name[i].setTextureSize(120, 60);

		volume_bar[i].textureFromFile("texture/volume_bar.png", 0, 0, 0);
		volume_bar[i].setButtonSize(226, 48);
		volume_bar[i].setButtonColor(240, 255, 255);
		volume_bar[i].setPosition(434, 126 + 60 * i);

		volume_block[i].loadTextureFromFile("texture/volume_block.png", 0, 0, 0);
		volume_block[i].setTextureSize(20, 40);
		volume_block[i].setTextureColor(240, 255, 255);
	}

	picker_surface = IMG_Load("texture/color_picker.png");
	for(int i = 0; i < 2; i++) {
		picker_name[i].loadTextureFromText(!i ? "Entity" : "Particle", deco_font, {240,255,255});
		picker_name[i].setTextureSize(!i ? 80 : 100, 50);

		picker_frame[i].loadTextureFromFile("texture/picker_frame.png", 255);
		picker_frame[i].setTextureSize(100, 100);
		picker_frame[i].setTextureColor(240, 255, 255);

		picker_sample[i].setButtonSize(76, 76);
		picker_sample[i].setPosition(287, 312 + 150 * i);

		color_picker[i].textureFromFile("texture/color_picker.png");
		color_picker[i].setButtonSize(64, 64);
		color_picker[i].setPosition(375, 318 + 150 * i);
	}

	model_frame.loadTextureFromFile("texture/model_frame.png", 255);
	model_frame.setTextureSize(210, 300);
	model_frame.setTextureColor(240, 255, 255);
	player_model.loadTextureFromFile("texture/player.png");
	player_model.setTextureSize(80, 100);
	player_model.setTextureColor(color_code[0][0], color_code[0][1], color_code[0][2]);
	particle_model.loadParticleTexture("texture/player_particle.png");
	particle_model.setParticleSize(20, 20);
	particle_model.setParticleColor(color_code[1][0], color_code[1][1], color_code[1][2]);
}
#endif // !MEDIA_H
