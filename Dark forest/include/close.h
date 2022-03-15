//#pragma once
#ifndef CLOSE
#define CLOSE
#include<fstream>
#include"basicVariable.h"
#include"advancedVariable.h"
void close() {
	//close shared items
	menu_background.free();
	transition_screen.free();
	back.free();
	TTF_CloseFont(button_font);
	TTF_CloseFont(deco_font);
	button_font = deco_font = nullptr;
	Mix_FreeChunk(touch_button);
	Mix_FreeChunk(click_button);
	touch_button = click_button = nullptr;

	//close menu
	menu_title.free();
	play.free();
	instruction.free();
	high_score.free();
	Mix_FreeMusic(menu_music);
	menu_music = nullptr;

	//close game play
	game_background.free();
	game_map.free();
	support_board.free();
	dead_background.free();
	platform.clear();

	player.free();

	skull_warning.free();
	for(auto& e : skull) e.free();
	skull.clear();
	skull_curse.clear();
	exploded_texture.free();
	Mix_FreeChunk(exploded_sound);
	exploded_sound = nullptr;

	reaper_warning.free();
	while(!reaper_death.empty()) {
		get<0>(reaper_death[0]).free();
		reaper_death.erase(begin(reaper_death));
	}
	for(auto& e : reaper) e.free();
	reaper.clear();
	reaper_curse.clear();
	shadow_caught_texture.free();
	Mix_FreeChunk(shadow_caught_sound);
	shadow_caught_sound = nullptr;

	pause_game.free();
	resume_game.free();
	exit_game.free();

	//close instruction

	//close high score
	std::ofstream new_data("high_score_data.txt");
	for(auto& sd : score_data) if(sd.first != "") new_data << sd.first << "\n" << sd.second << "\n";
	score_data.clear();
	resetScore.free();

	//close renderer and window
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = nullptr;
	window = nullptr;

	//close all tool
	IMG_Quit();
	TTF_Quit();
	Mix_Quit();
	SDL_Quit();
}
#endif // !CLOSE
