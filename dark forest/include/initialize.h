//#pragma once
#ifndef INIT
#define INIT
#include"basicVariable.h"
void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark forest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

	std::ifstream old_setting_data("game data/setting data.txt");
	old_setting_data >> music_volume;
	old_setting_data >> touch_button_volume >> click_button_volume;
	old_setting_data >> player_move_volume;
	old_setting_data >> skuLL_move_volume >> exploded_volume;
	old_setting_data >> reaper_move_volume >> reaper_die_volume >> shadow_caught_volume;

	std::ifstream old_score_data("game data/score data.txt");
	std::string used_name;
	Uint8 played_mode;
	Uint32 survived_time;
	while(!old_score_data.eof()) {
		std::getline(old_score_data, used_name);
		old_score_data >> played_mode >> survived_time >> std::ws;
		if(!used_name.empty()) score_data.emplace_back(used_name, played_mode, survived_time);
	}
	score_data.resize(5);
}
#endif // !INIT
