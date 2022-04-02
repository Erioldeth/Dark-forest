#ifndef CLOSE_H
#define CLOSE_H
#include"advancedVariable.h"
void close() {
	//close shared items
	menu_background.free();
	transition_screen.free();
	back.free();
	reset.free();
	TTF_CloseFont(button_font);
	TTF_CloseFont(deco_font);
	button_font = deco_font = nullptr;
	Mix_FreeChunk(touch_button);
	Mix_FreeChunk(click_button);
	touch_button = click_button = nullptr;



	//close menu
	game_title.free();
	play.free();
	setting.free();
	high_score.free();
	Mix_FreeMusic(menu_music);
	menu_music = nullptr;



	//close game play
	game_background.free();
	game_map.free();
	support_board.free();
	support_frame.free();
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
	for(auto& e : reaper) e.free();
	reaper.clear();
	reaper_curse.clear();
	for(auto& rd : reaper_death) get<0>(rd).free();
	reaper_death.clear();
	shadow_caught_texture.free();
	Mix_FreeChunk(shadow_caught_sound);
	shadow_caught_sound = nullptr;

	easy_mode.free();
	normal_mode.free();
	hard_mode.free();
	pause_game.free();
	resume_game.free();
	exit_game.free();



	//close setting
	std::ofstream new_setting("game data/setting data.txt");
	for(auto& v : volume_ratio) new_setting << v << ' ';
	volume_ratio.clear();
	new_setting << "\n";
	for(auto& code : color_code) {
		for(auto& c : code) new_setting << c << ' ';
		code.clear();
		new_setting << "\n";
	}
	color_code.clear();

	for(auto& v : volume_name) v.free();
	volume_name.clear();
	for(auto& v : volume_bar) v.free();
	volume_bar.clear();
	for(auto& v : volume_block) v.free();
	volume_block.clear();
	touch_bar.clear();

	SDL_FreeSurface(picker_surface);
	picker_surface = nullptr;
	for(auto& p : picker_name) p.free();
	picker_name.clear();
	for(auto& p : picker_frame) p.free();
	picker_frame.clear();
	for(auto& p : picker_sample) p.free();
	picker_sample.clear();
	for(auto& p : color_picker) p.free();
	color_picker.clear();
	touch_sample.clear();
	picker_activated.clear();

	model_frame.free();
	player_model.free();
	particle_model.free();



	//close score
	std::ofstream new_score("game data/score data.txt");
	for(auto& sd : score_data) if(!get<0>(sd).empty()) new_score << get<0>(sd) << "\n" << get<1>(sd) << ' ' << get<2>(sd) << "\n";
	score_data.clear();



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
#endif // !CLOSE_H
