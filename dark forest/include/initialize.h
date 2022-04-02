#ifndef INIT_H
#define INIT_H
#include"basicVariable.h"
void init() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	window = SDL_CreateWindow("Dark forest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);


	std::ifstream old_score("game data/score data.txt");
	std::string used_name;
	Uint8 played_mode;
	Uint32 survived_time;
	while(!old_score.eof()) {
		std::getline(old_score, used_name);
		old_score >> played_mode >> survived_time >> std::ws;
		if(!used_name.empty()) score_data.emplace_back(used_name, played_mode, survived_time);
	}
	score_data.resize(5);


	std::ifstream old_setting("game data/setting data.txt");
	for(auto& v : volume_ratio) old_setting >> v;
	for(auto& code : color_code) for(auto& c : code) old_setting >> c;
}
#endif // !INIT_H
