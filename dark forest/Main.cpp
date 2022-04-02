#include<sstream>
#include<algorithm>
#include"initialize.h"
#include"loadMedia.h"
#include"close.h"
void resetGame() {
	mode = NONE;

	for(int i = 0; i < CURRENT_SKULL; i++) skull[i].resetEntity();

	for(int i = 0; i < CURRENT_REAPER; i++) reaper[i].resetEntity();

	for(auto& rd : reaper_death) get<0>(rd).free();
	reaper_death.clear();

	player.resetEntity();

	player_name.clear();
	survival_time = 0;
	PAUSED = EXPLODED = SHADOW_CAUGHT = DEAD = false;
}

void updateScoreData() {
	if(!player_name.empty() && survival_time >= 1) {
		score_data.emplace_back(player_name, mode, survival_time);
		sort(begin(score_data), end(score_data), [](std::tuple<std::string, Uint8, Uint32> a, std::tuple<std::string, Uint8, Uint32> b) {
			if(get<1>(a) != get<1>(b)) return get<1>(a) > get<1>(b);
			return get<2>(a) > get<2>(b);
			 });
		score_data.resize(5);
	}
}

std::string timeFormat(Uint32 time) {
	std::string min = std::to_string(time / 60), sec = std::to_string(time % 60);
	if(min.length() < 2) min = '0' + min;
	if(sec.length() < 2) sec = '0' + sec;
	return min + ':' + sec;
}

void showTime() {
	std::stringstream timeText;
	timeText.str("");
	timeText << timeFormat(survive_clock.getTick() / 1000);
	Texture timeTexture;
	timeTexture.loadTextureFromText(timeText.str(), deco_font, {255,255,0});
	timeTexture.setTextureSize(90, 45);
	timeTexture.renderTexture((SCREEN_WIDTH - timeTexture.textureWidth()) >> 1, 10);
	timeTexture.free();
}

SDL_Color getColor(const int x, const int y) {
	Uint32 pixel_data = *(Uint32*)((Uint8*)picker_surface->pixels + y * picker_surface->pitch + x * picker_surface->format->BytesPerPixel);
	SDL_Color color{0,0,0,255};
	SDL_GetRGB(pixel_data, picker_surface->format, &color.r, &color.g, &color.b);
	return color;
}

int main(int argc, char** argv) {
	init();
	loadMedia();
	SDL_Event e;
	while(true) switch(option) {
		case MENU:
			if(!Mix_PlayingMusic()) {
				Mix_FreeMusic(menu_music);
				menu_music = nullptr;
				std::uniform_int_distribution<> file(1, 3);
				std::string path = "sound/menu_music" + std::to_string(file(mt)) + ".wav";
				menu_music = Mix_LoadMUS(path.c_str());
				Mix_VolumeMusic(5 * volume_ratio[0]);
				Mix_PlayMusic(menu_music, 1);
			}
			while(option == MENU) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					play.handleEvent(&e);
					setting.handleEvent(&e);
					high_score.handleEvent(&e);
				}
				switch(play.getCurrentState()) {
					case MOUSE_OUT:
						touch_play = false;
						play.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_play = true;
						play.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						option = PLAY;
						play.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(setting.getCurrentState()) {
					case MOUSE_OUT:
						touch_setting = false;
						setting.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_setting = true;
						setting.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						option = SETTING;
						setting.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(high_score.getCurrentState()) {
					case MOUSE_OUT:
						touch_high_score = false;
						high_score.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_high_score = true;
						high_score.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						option = HIGH_SCORE;
						high_score.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				if(option == MENU) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);
					menu_background.renderTexture(0, 0);
					game_title.renderTexture((SCREEN_WIDTH - game_title.textureWidth()) >> 1, 25);
					play.renderButton();
					setting.renderButton();
					high_score.renderButton();
					SDL_RenderPresent(renderer);
					if(touch_play || touch_setting || touch_high_score) {
						if(!lock_menu_button_sound) {
							lock_menu_button_sound = true;
							Mix_PlayChannel(1, touch_button, 0);
						}
					}
					else lock_menu_button_sound = false;
				}
			}
			break;

		case PLAY:
			Mix_FadeOutMusic(1500);

			for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close(); return 0;
				}

				transition_screen.setTextureAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				menu_background.renderTexture(0, 0);
				game_title.renderTexture((SCREEN_WIDTH - game_title.textureWidth()) >> 1, 25);

				play.renderButton();
				setting.renderButton();
				high_score.renderButton();

				transition_screen.renderTexture(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			while(mode == NONE) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					easy_mode.handleEvent(&e);
					normal_mode.handleEvent(&e);
					hard_mode.handleEvent(&e);
				}
				switch(easy_mode.getCurrentState()) {
					case MOUSE_OUT:
						touch_easy = false;
						easy_mode.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_easy = true;
						easy_mode.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						mode = EASY;
						CURRENT_SKULL = 4;
						CURRENT_REAPER = 2;
						easy_mode.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(normal_mode.getCurrentState()) {
					case MOUSE_OUT:
						touch_normal = false;
						normal_mode.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_normal = true;
						normal_mode.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						mode = NORMAL;
						CURRENT_SKULL = 6;
						CURRENT_REAPER = 3;
						normal_mode.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(hard_mode.getCurrentState()) {
					case MOUSE_OUT:
						touch_hard = false;
						hard_mode.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_hard = true;
						hard_mode.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						mode = HARD;
						CURRENT_SKULL = 8;
						CURRENT_REAPER = 4;
						hard_mode.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				if(mode == NONE) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);
					easy_mode.renderButton();
					normal_mode.renderButton();
					hard_mode.renderButton();
					SDL_RenderPresent(renderer);
					if(touch_easy || touch_normal || touch_hard) {
						if(!lock_mode_button_sound) {
							lock_mode_button_sound = true;
							Mix_PlayChannel(1, touch_button, 0);
						}
					}
					else lock_mode_button_sound = false;
				}
			}

			for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close(); return 0;
				}

				transition_screen.setTextureAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				easy_mode.renderButton();
				normal_mode.renderButton();
				hard_mode.renderButton();

				transition_screen.renderTexture(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close(); return 0;
				}

				transition_screen.setTextureAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				game_background.renderTexture(0, 0);
				game_map.renderTexture(0, 0);

				player.renderEntity();

				showTime();

				transition_screen.renderTexture(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			while(!DEAD && option == PLAY) {
				if(!survive_clock.isStarted()) survive_clock.start();
				while(!reaper_death.empty() && get<2>(reaper_death[0]) == 200) {
					get<0>(reaper_death[0]).free();
					reaper_death.erase(begin(reaper_death));
				}
				if(PAUSED) {
					survive_clock.pause();

					while(SDL_PollEvent(&e) != 0) {
						if(e.type == SDL_QUIT) {
							close(); return 0;
						}
						else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
							Mix_PlayChannel(1, click_button, 0);
							PAUSED = false;
							pause_game.reset();
						}
						player.handleEvent(&e);
						resume_game.handleEvent(&e);
						exit_game.handleEvent(&e);
					}

					switch(resume_game.getCurrentState()) {
						case MOUSE_OUT:
							touch_resume = false;
							resume_game.setButtonColor(255, 255, 255);
							break;
						case MOUSE_IN:
							touch_resume = true;
							resume_game.setButtonColor(255, 0, 0);
							break;
						case MOUSE_DOWN:
							PAUSED = false;
							resume_game.reset();
							Mix_PlayChannel(1, click_button, 0);
							break;
					}
					switch(exit_game.getCurrentState()) {
						case MOUSE_OUT:
							touch_exit = false;
							exit_game.setButtonColor(255, 255, 255);
							break;
						case MOUSE_IN:
							touch_exit = true;
							exit_game.setButtonColor(255, 0, 0);
							break;
						case MOUSE_DOWN:
							option = MENU;
							exit_game.reset();
							Mix_PlayChannel(1, click_button, 0);
							break;
					}

					if(PAUSED && option == PLAY) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.renderTexture(0, 0);
						game_map.renderTexture(0, 0);

						for(auto& t : reaper_death) {
							int frame = get<2>(t) / 10;
							SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
							get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
						}

						player.renderEntity();
						for(int i = 0; i < CURRENT_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < CURRENT_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}

						support_board.renderTexture((SCREEN_WIDTH - support_board.textureWidth()) >> 1, (SCREEN_HEIGHT - support_board.textureHeight()) >> 1);
						support_frame.renderTexture((SCREEN_WIDTH - support_frame.textureWidth()) >> 1, (SCREEN_HEIGHT - support_frame.textureHeight()) >> 1);
						resume_game.renderButton();
						exit_game.renderButton();

						showTime();

						SDL_RenderPresent(renderer);

						if(touch_resume || touch_exit) {
							if(!lock_resume_and_exit_button_sound) {
								lock_resume_and_exit_button_sound = true;
								Mix_PlayChannel(1, touch_button, 0);
							}
						}
						else lock_resume_and_exit_button_sound = false;
					}
				}
				else {
					survive_clock.resume();

					while(SDL_PollEvent(&e) != 0) {
						if(e.type == SDL_QUIT) {
							close(); return 0;
						}
						else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
							Mix_HaltChannel(-1);
							Mix_PlayChannel(1, click_button, 0);
							PAUSED = true;
							resume_game.reset();
							exit_game.reset();
						}
						player.handleEvent(&e);
						pause_game.handleEvent(&e);
					}

					switch(pause_game.getCurrentState()) {
						case MOUSE_OUT:
							touch_pause = false;
							pause_game.setButtonColor(255, 255, 255);
							break;
						case MOUSE_IN:
							touch_pause = true;
							pause_game.setButtonColor(255, 0, 0);
							break;
						case MOUSE_DOWN:
							PAUSED = true;
							pause_game.reset();
							Mix_HaltChannel(-1);
							Mix_PlayChannel(1, click_button, 0);
							break;
					}

					if(!PAUSED) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.renderTexture(0, 0);
						game_map.renderTexture(0, 0);

						for(auto& t : reaper_death) {
							int frame = get<2>(t) / 10;
							SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
							get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
							get<2>(t)++;
						}

						player.move(platform);
						player.renderEntity();

						for(int i = 0; i < CURRENT_SKULL; i++) {
							if(skull_curse[i]) {
								skull[i].move(player);
								skull[i].renderEntity();
							}
							else {
								skull[i].alert();
								skull_curse[i] = skull_creation();
							}
						}
						for(int i = 0; i < CURRENT_REAPER; i++) {
							if(reaper_curse[i]) {
								reaper[i].move(player);
								reaper[i].renderEntity();
							}
							else {
								reaper[i].alert();
								reaper_curse[i] = reaper_creation();
							}
						}

						pause_game.renderButton();

						showTime();

						SDL_RenderPresent(renderer);

						if(touch_pause) {
							if(!lock_pause_button_sound) {
								lock_pause_button_sound = true;
								Mix_PlayChannel(1, touch_button, 0);
							}
						}
						else lock_pause_button_sound = false;
					}
				}
			}

			survive_clock.pause();
			while(!reaper_death.empty() && get<2>(reaper_death[0]) == 200) {
				get<0>(reaper_death[0]).free();
				reaper_death.erase(begin(reaper_death));
			}

			if(option == PLAY) {
				option = MENU;
				Mix_HaltChannel(-1);

				if(SHADOW_CAUGHT) {
					Mix_PlayChannel(3, shadow_caught_sound, 0);
					for(int i = 0; i < 720; i++) {
						if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
							close(); return 0;
						}

						SDL_Rect shadowSprite = {100 * ((i / 20) % 6),100 * ((i / 20) / 6),100,100};

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.renderTexture(0, 0);
						game_map.renderTexture(0, 0);

						for(auto& t : reaper_death) {
							int frame = get<2>(t) / 10;
							SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
							get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
						}

						if(i < 240) player.renderEntity();

						shadow_caught_texture.renderTexture(player.entityPosition().x - ((shadow_caught_texture.textureWidth() - player.entityWidth()) >> 1), player.entityPosition().y - ((shadow_caught_texture.textureHeight() - player.entityHeight()) >> 1), &shadowSprite);

						for(int j = 0; j < CURRENT_SKULL; j++) {
							if(skull_curse[j]) skull[j].renderEntity();
							else skull[j].alert();
						}
						for(int j = 0; j < CURRENT_REAPER; j++) {
							if(reaper_curse[j]) reaper[j].renderEntity();
							else reaper[j].alert();
						}

						showTime();

						SDL_RenderPresent(renderer);
					}
				}
				else if(EXPLODED) {
					Mix_PlayChannel(4, exploded_sound, 0);
					for(int i = 0; i < 280; i++) {
						if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
							close(); return 0;
						}

						SDL_Rect explodeSprite = {128 * ((i / 20) & 3),128 * ((i / 20) >> 2),128,128};

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.renderTexture(0, 0);
						game_map.renderTexture(0, 0);

						for(auto& t : reaper_death) {
							int frame = get<2>(t) / 10;
							SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
							get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
						}

						exploded_texture.renderTexture(player.entityPosition().x - ((exploded_texture.textureWidth() - player.entityWidth()) >> 1), player.entityPosition().y - ((exploded_texture.textureHeight() - player.entityHeight()) >> 1), &explodeSprite);

						for(int j = 0; j < CURRENT_SKULL; j++) {
							if(skull_curse[j]) skull[j].renderEntity();
							else skull[j].alert();
						}
						for(int j = 0; j < CURRENT_REAPER; j++) {
							if(reaper_curse[j]) reaper[j].renderEntity();
							else reaper[j].alert();
						}

						showTime();

						SDL_RenderPresent(renderer);
					}
				}
				while(Mix_Playing(3) || Mix_Playing(4)) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.renderTexture(0, 0);
					game_map.renderTexture(0, 0);

					for(auto& t : reaper_death) {
						int frame = get<2>(t) / 10;
						SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
						get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
					}

					for(int j = 0; j < CURRENT_SKULL; j++) {
						if(skull_curse[j]) skull[j].renderEntity();
						else skull[j].alert();
					}
					for(int j = 0; j < CURRENT_REAPER; j++) {
						if(reaper_curse[j]) reaper[j].renderEntity();
						else reaper[j].alert();
					}

					showTime();

					SDL_RenderPresent(renderer);
				}

				survival_time = survive_clock.getTick() / 1000;

				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					dead_background.setTextureAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.renderTexture(0, 0);
					game_map.renderTexture(0, 0);

					for(auto& t : reaper_death) {
						int frame = get<2>(t) / 10;
						SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
						get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
					}

					for(int i = 0; i < CURRENT_SKULL; i++) {
						if(skull_curse[i]) skull[i].renderEntity();
						else skull[i].alert();
					}
					for(int i = 0; i < CURRENT_REAPER; i++) {
						if(reaper_curse[i]) reaper[i].renderEntity();
						else reaper[i].alert();
					}

					showTime();

					dead_background.renderTexture(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					dead_background.setTextureAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					dead_background.renderTexture(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				bool confirmedName = false;
				SDL_StartTextInput();
				while(!confirmedName) {
					while(SDL_PollEvent(&e) != 0) switch(e.type) {
						case SDL_QUIT:
							close(); return 0;
						case SDL_KEYDOWN:
							switch(e.key.keysym.sym) {
								case SDLK_BACKSPACE:
									if(!player_name.empty()) player_name.pop_back();
									break;
								case SDLK_c:
									if(SDL_GetModState() && KMOD_CTRL) SDL_SetClipboardText(player_name.c_str());
									break;
								case SDLK_x:
									if(SDL_GetModState() && KMOD_CTRL) {
										SDL_SetClipboardText(player_name.c_str());
										player_name.clear();
									}
									break;
								case SDLK_v:
									if(SDL_GetModState() && KMOD_CTRL) player_name.append(SDL_GetClipboardText());
									break;
								case SDLK_RETURN:
									confirmedName = true;
									break;
							}
							break;
						case SDL_TEXTINPUT:
							if(!(SDL_GetModState() && KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'x' || e.text.text[0] == 'X' || e.text.text[0] == 'v' || e.text.text[0] == 'V'))) player_name.append(e.text.text);
							break;
					}
					if(!confirmedName) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						if(player_name == "") player_name = " ";
						Texture nameTexture, instructTexture;

						instructTexture.loadTextureFromText("Enter your name", deco_font, {255,0,0});
						instructTexture.setTextureSize(450, 100);
						instructTexture.renderTexture((SCREEN_WIDTH - instructTexture.textureWidth()) >> 1, 210);

						nameTexture.loadTextureFromText(player_name, deco_font, {91,44,111});
						nameTexture.setTextureSize(30 * (int)player_name.length(), 100);
						nameTexture.renderTexture((SCREEN_WIDTH - nameTexture.textureWidth()) >> 1, 330);

						SDL_RenderPresent(renderer);

						if(player_name == " ") player_name = "";
						nameTexture.free();
						instructTexture.free();
					}
				}
				SDL_StopTextInput();

				while(!player_name.empty() && player_name[0] == ' ') player_name.erase(0, 1);
				while(!player_name.empty() && player_name.back() == ' ') player_name.pop_back();

				updateScoreData();

				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setTextureAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					if(player_name == "") player_name = " ";
					Texture textTexture, instructTexture;

					instructTexture.loadTextureFromText("Enter your name", deco_font, {255,0,0});
					instructTexture.setTextureSize(450, 100);
					instructTexture.renderTexture((SCREEN_WIDTH - instructTexture.textureWidth()) >> 1, 210);

					textTexture.loadTextureFromText(player_name, deco_font, {91,44,111});
					textTexture.setTextureSize(30 * (int)player_name.length(), 100);
					textTexture.renderTexture((SCREEN_WIDTH - textTexture.textureWidth()) >> 1, 330);

					transition_screen.renderTexture(0, 0);

					SDL_RenderPresent(renderer);

					if(player_name == " ") player_name = "";
					textTexture.free();
					instructTexture.free();

					SDL_Delay(50);
				}
				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setTextureAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					menu_background.renderTexture(0, 0);
					game_title.renderTexture((SCREEN_WIDTH - game_title.textureWidth()) >> 1, 25);

					play.renderButton();
					setting.renderButton();
					high_score.renderButton();

					transition_screen.renderTexture(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);
			}
			else {
				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setTextureAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.renderTexture(0, 0);
					game_map.renderTexture(0, 0);

					for(auto& t : reaper_death) {
						int frame = get<2>(t) / 10;
						SDL_Rect death_sprite = {192 * (frame % 5),192 * (frame / 5),192,192};
						get<0>(t).renderTexture(get<1>(t).x, get<1>(t).y, &death_sprite);
					}

					player.renderEntity();
					for(int i = 0; i < CURRENT_SKULL; i++) {
						if(skull_curse[i]) skull[i].renderEntity();
						else skull[i].alert();
					}
					for(int i = 0; i < CURRENT_REAPER; i++) {
						if(reaper_curse[i]) reaper[i].renderEntity();
						else reaper[i].alert();
					}

					showTime();

					transition_screen.renderTexture(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setTextureAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					menu_background.renderTexture(0, 0);
					game_title.renderTexture((SCREEN_WIDTH - game_title.textureWidth()) >> 1, 25);

					play.renderButton();
					setting.renderButton();
					high_score.renderButton();

					transition_screen.renderTexture(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);
			}
			survive_clock.stop();
			resetGame();
			break;

		case SETTING:
			model_movement = 0;
			particle_model.reset();
			for(auto p : picker_activated) p = false;
			while(option == SETTING) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					back.handleEvent(&e);
					reset.handleEvent(&e);
					for(auto& v : volume_bar) v.handleEvent(&e);
					for(auto& p : picker_sample) p.handleEvent(&e);
					for(int i = 0; i < 2; i++) if(picker_activated[i]) color_picker[i].handleEvent(&e);
				}
				switch(back.getCurrentState()) {
					case MOUSE_OUT:
						touch_back = false;
						back.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_back = true;
						back.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						option = MENU;
						back.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(reset.getCurrentState()) {
					case MOUSE_OUT:
						touch_reset = false;
						reset.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_reset = true;
						reset.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						reset.setButtonState(MOUSE_IN);
						for(auto& v : volume_ratio) v = 10;
						for(auto& code : color_code) for(auto& c : code) c = 255;
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				for(int i = 0; i < 2; i++) {
					switch(volume_bar[i].getCurrentState()) {
						case MOUSE_OUT:
							touch_bar[i] = false;
							volume_name[i].setTextureColor(240, 255, 255);
							volume_bar[i].setButtonColor(240, 255, 255);
							volume_block[i].setTextureColor(240, 255, 255);
							break;
						case MOUSE_IN:
							touch_bar[i] = true;
							volume_name[i].setTextureColor(147, 112, 219);
							volume_bar[i].setButtonColor(147, 112, 219);
							break;
						case MOUSE_DOWN:
							int x;
							SDL_GetMouseState(&x, nullptr);
							volume_ratio[i] = (int)ceil(double(x - 438) / 22.0);
							volume_bar[i].setButtonState(MOUSE_IN);
							Mix_PlayChannel(1, click_button, 0);
							break;
					}
					if(volume_bar[i].getCurrentState() == MOUSE_IN) switch(volume_ratio[i]) {
						case 1: case 2: case 3: volume_block[i].setTextureColor(255, 49, 49); break;
						case 8: case 9: case 10: volume_block[i].setTextureColor(15, 255, 80); break;
						default: volume_block[i].setTextureColor(255, 240, 31); break;
					}
					if(!i) Mix_VolumeMusic(5 * volume_ratio[i]);
					else {
						Mix_VolumeChunk(touch_button, 10 * volume_ratio[i]);
						Mix_VolumeChunk(click_button, 10 * volume_ratio[i]);
						player.loadMoveSound("sound/player_move.wav", 10 * volume_ratio[i]);
						Mix_VolumeChunk(exploded_sound, 10 * volume_ratio[i]);
						for(auto& e : reaper) e.loadDeadSound("sound/reaper_death.wav", 3 * volume_ratio[i]);
						Mix_VolumeChunk(shadow_caught_sound, 10 * volume_ratio[i]);
					}

					switch(picker_sample[i].getCurrentState()) {
						case MOUSE_OUT:
							touch_sample[i] = false;
							picker_name[i].setTextureColor(240, 255, 255);
							picker_frame[i].setTextureColor(240, 255, 255);
							break;
						case MOUSE_IN:
							touch_sample[i] = true;
							picker_name[i].setTextureColor(147, 112, 219);
							picker_frame[i].setTextureColor(147, 112, 219);
							break;
						case MOUSE_DOWN:
							picker_activated[i] = true;
							picker_sample[i].setButtonState(MOUSE_IN);
							Mix_PlayChannel(1, click_button, 0);
							break;
					}
					if(picker_activated[i]) switch(color_picker[i].getCurrentState()) {
						case MOUSE_OUT:
							touch_picker[i] = false;
							break;
						case MOUSE_IN:
							touch_picker[i] = true;
							break;
						case MOUSE_DOWN:
							int x, y;
							SDL_GetMouseState(&x, &y);
							SDL_Color new_color = getColor((x - 293) << 2, (y - (318 + 150 * i)) << 2);
							color_code[i][0] = new_color.r;
							color_code[i][1] = new_color.g;
							color_code[i][2] = new_color.b;
							color_picker[i].setButtonState(MOUSE_OUT);
							touch_picker[i] = false;
							picker_activated[i] = false;
							Mix_PlayChannel(1, click_button, 0);
							break;
					}
					player.setEntityColor(color_code[0][0], color_code[0][1], color_code[0][2]);
					player_model.setTextureColor(color_code[0][0], color_code[0][1], color_code[0][2]);
					player.setParticleColor(color_code[1][0], color_code[1][1], color_code[1][2]);
					particle_model.setParticleColor(color_code[1][0], color_code[1][1], color_code[1][2]);
				}
				if(option == SETTING) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);
					menu_background.renderTexture(0, 0);
					back.renderButton();
					reset.renderButton();
					for(int i = 0; i < 2; i++) {
						volume_name[i].renderTexture(300, 120 + i * 60);
						volume_bar[i].renderButton();
						for(int j = 0; j < volume_ratio[i]; j++) volume_block[i].renderTexture(438 + j * 22, 130 + i * 60);

						SDL_Rect picker_sample_region{287,312 + 150 * i,76,76};
						SDL_SetRenderDrawColor(renderer, color_code[i][0], color_code[i][1], color_code[i][2], 255);
						SDL_RenderFillRect(renderer, &picker_sample_region);
						picker_name[i].renderTexture(275 + ((100 - picker_name[i].textureWidth()) >> 1), 400 + 150 * i);
						picker_frame[i].renderTexture(275, 300 + 150 * i);
						if(picker_activated[i]) color_picker[i].renderButton();
					}

					model_frame.renderTexture(475, 300);

					SDL_Rect particle_model_region{555,425,50,50};
					particle_model.renderParticle(particle_model_region, 40, 8, 70, 70, 15);

					SDL_Rect movement{96 * (model_movement / 5) + 32, 84 + 42, 32, 42};
					player_model.renderTexture(540, 400, &movement, 0.0, nullptr, SDL_FLIP_HORIZONTAL);
					model_movement = (model_movement + 1) % 35;

					SDL_RenderPresent(renderer);
					if(touch_back || touch_reset || touch_bar[0] || touch_bar[1] || touch_sample[0] || touch_sample[1] || touch_picker[0] || touch_picker[1]) {
						if(!lock_setting_button_sound) {
							lock_setting_button_sound = true;
							Mix_PlayChannel(1, touch_button, 0);
						}
					}
					else lock_setting_button_sound = false;
				}
			}
			break;

		case HIGH_SCORE:
			Texture info_texture;
			while(option == HIGH_SCORE) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					back.handleEvent(&e);
					reset.handleEvent(&e);
				}
				switch(back.getCurrentState()) {
					case MOUSE_OUT:
						touch_back = false;
						back.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_back = true;
						back.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						option = MENU;
						back.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(reset.getCurrentState()) {
					case MOUSE_OUT:
						touch_reset = false;
						reset.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_reset = true;
						reset.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						reset.setButtonState(MOUSE_IN);
						score_data.clear();
						score_data.resize(5);
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				if(option == HIGH_SCORE) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);
					menu_background.renderTexture(0, 0);
					back.renderButton();
					reset.renderButton();
					for(int i = 0; i < 5; i++) {
						std::string info_name = "<name>", info_mode = "<mode>", info_time = "<00:00>";
						SDL_Color text_color;
						switch(i) {
							case 0: text_color = {112,41,99}; break;
							case 1:	text_color = {220,20,60}; break;
							case 2: text_color = {255,192,0}; break;
							case 3: text_color = {192,192,192}; break;
							case 4: text_color = {205, 127, 50}; break;
						}
						if(get<2>(score_data[i]) != 0) {
							info_name = get<0>(score_data[i]);
							if(info_name.length() > 10) info_name = std::string(info_name, 0, 7) + "...";

							switch(get<1>(score_data[i])) {
								case EASY: info_mode = "easy"; break;
								case NORMAL: info_mode = "normal"; break;
								case HARD: info_mode = "hard"; break;
							}

							info_time = timeFormat(get<2>(score_data[i]));
						}
						info_texture.loadTextureFromText(info_name, deco_font, text_color);
						info_texture.setTextureSize(20 * (int)info_name.length(), 80);
						info_texture.renderTexture(150 + ((200 - info_texture.textureWidth()) >> 1), 120 + 80 * i);

						info_texture.loadTextureFromText(info_mode, deco_font, text_color);
						info_texture.setTextureSize(20 * (int)info_mode.length(), 80);
						info_texture.renderTexture(510 + ((120 - info_texture.textureWidth()) >> 1), 120 + 80 * i);

						info_texture.loadTextureFromText(info_time, deco_font, text_color);
						info_texture.setTextureSize(20 * (int)info_time.length(), 80);
						info_texture.renderTexture(710 + ((100 - info_texture.textureWidth()) >> 1), 120 + 80 * i);
					}
					SDL_RenderPresent(renderer);
					if(touch_back || touch_reset) {
						if(!lock_high_score_button_sound) {
							lock_high_score_button_sound = true;
							Mix_PlayChannel(1, touch_button, 0);
						}
					}
					else lock_high_score_button_sound = false;
				}
			}
			info_texture.free();
			break;
	}
	close();
	return 0;
}
