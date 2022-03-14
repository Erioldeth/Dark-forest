#include<iostream>
#include<sstream>
#include<algorithm>
#include"initialize.h"
#include"loadMedia.h"
#include"close.h"

void resetGame() {
	for(int i = 0; i < MAX_SKULL; i++) {
		skull_curse[i] = false;
		skull[i].setVeclocity();
		skull[i].setPosition();
		skull[i].resetEntity();
	}
	for(int i = 0; i < MAX_REAPER; i++) {
		reaper_curse[i] = false;
		reaper[i].setVeclocity();
		reaper[i].setPosition();
		reaper[i].resetEntity();
	}

	player.setPosition((SCREEN_WIDTH - player.entityWidth()) >> 1, (SCREEN_HEIGHT - player.entityHeight()) >> 1);
	player.resetEntity();

	player_name = "";
	survival_time = 0;
	PAUSED = EXPLODED = SHADOW_CAUGHT = DEAD = false;
}

void inputScoreData() {
	std::ifstream old_data("high_score_data.txt");
	std::string data_name;
	Uint32 data_time;
	while(!old_data.eof()) {
		std::getline(old_data, data_name);
		old_data >> data_time >> std::ws;
		if(!data_name.empty()) score_data.emplace_back(data_name, data_time);
	}
	score_data.resize(5);
}

void updateScoreData() {
	if(player_name != "") {
		score_data.emplace_back(player_name, survival_time);
		sort(begin(score_data), end(score_data), [](std::pair<std::string, Uint32> a, std::pair<std::string, Uint32> b) {return a.second > b.second; });
		score_data.resize(5);
	}
}

std::string timeFormat(Uint32 time) {
	time /= 1000;
	std::string min = std::to_string(time / 60), sec = std::to_string(time % 60);
	if(min.length() < 2) min = '0' + min;
	if(sec.length() < 2) sec = '0' + sec;
	return min + ':' + sec;
}

void showTime() {
	std::stringstream timeText;
	timeText.str("");
	timeText << timeFormat(survive_clock.getTick());
	Texture timeTexture;
	timeTexture.loadFromRenderedText(timeText.str(), deco_font, {255,255,0});
	timeTexture.setSize(90, 45);
	timeTexture.render((SCREEN_WIDTH - timeTexture.getWidth()) >> 1, 10);
	timeTexture.free();
}

int main(int argc, char** argv) {
	init();
	loadMedia();
	inputScoreData();
	SDL_Event e;
	while(true) switch(option) {
		//menu part
		case MENU:
			if(Mix_PlayingMusic() == 0) Mix_PlayMusic(menu_music, 1);
			while(option == MENU) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					play.handleEvent(&e);
					instruction.handleEvent(&e);
					high_score.handleEvent(&e);
				}
				switch(play.getCurrentSprite()) {
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
				switch(instruction.getCurrentSprite()) {
					case MOUSE_OUT:
						touch_instruction = false;
						instruction.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_instruction = true;
						instruction.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						option = INSTRUCTION;
						instruction.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				switch(high_score.getCurrentSprite()) {
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
					menu_background.render(0, 0);
					menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) >> 1, 25);
					play.renderButton();
					instruction.renderButton();
					high_score.renderButton();
					SDL_RenderPresent(renderer);
					if(touch_play || touch_instruction || touch_high_score) {
						if(!lock_menu_button_sound) {
							lock_menu_button_sound = true;
							Mix_PlayChannel(1, touch_button, 0);
						}
					}
					else lock_menu_button_sound = false;
				}
			}
			break;

		//game play part
		case PLAY:
			resetGame();
			pause_game.setButtonColor(255, 255, 255);

			Mix_FadeOutMusic(1500);

			for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close(); return 0;
				}

				transition_screen.setAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				menu_background.render(0, 0);
				menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) >> 1, 25);

				play.renderButton();
				instruction.renderButton();
				high_score.renderButton();

				transition_screen.render(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
				if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
					close(); return 0;
				}

				transition_screen.setAlpha(alpha);

				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
				SDL_RenderClear(renderer);

				game_background.render(0, 0);
				game_map.render(0, 0);

				player.renderEntity();

				pause_game.renderButton();

				showTime();

				transition_screen.render(0, 0);

				SDL_RenderPresent(renderer);

				SDL_Delay(50);
			}
			SDL_Delay(500);

			while(!DEAD && option == PLAY) {
				if(!survive_clock.isStarted()) survive_clock.start();
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
						resume_game.handleEvent(&e);
						exit_game.handleEvent(&e);
					}

					switch(resume_game.getCurrentSprite()) {
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
					switch(exit_game.getCurrentSprite()) {
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

						game_background.render(0, 0);
						game_map.render(0, 0);

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}
						player.renderEntity();

						support_board.render((SCREEN_WIDTH - support_board.getWidth()) >> 1, (SCREEN_HEIGHT - support_board.getHeight()) >> 1);
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
							Mix_PlayChannel(1, click_button, 0);
							PAUSED = true;
							resume_game.reset();
							exit_game.reset();
						}
						player.handleEvent(&e);
						pause_game.handleEvent(&e);
					}

					switch(pause_game.getCurrentSprite()) {
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
							Mix_PlayChannel(1, click_button, 0);
							break;
					}

					if(!PAUSED) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);

						player.move(platform, player);
						player.renderEntity();

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) {
								skull[i].move(platform, player);
								skull[i].renderEntity();
							}
							else {
								skull[i].alert();
								skull_curse[i] = skull_creation();
							}
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) {
								reaper[i].move(platform, player);
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

			if(option == PLAY) {
				Mix_HaltChannel(-1);
				if(SHADOW_CAUGHT) {
					Mix_PlayChannel(3, shadow_caught_sound, 0);
					for(int i = 0; i < 36; i++) {
						if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
							close(); return 0;
						}

						SDL_Rect shadowSprite = {100 * (i % 6),100 * (i / 6),100,100};

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);

						if(i < 12) player.renderEntity();

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}

						shadow_caught_texture.render(player.entityPosition().x - ((shadow_caught_texture.getWidth() - player.entityWidth()) >> 1), player.entityPosition().y - ((shadow_caught_texture.getHeight() - player.entityHeight()) >> 1), &shadowSprite);

						showTime();

						SDL_RenderPresent(renderer);
						SDL_Delay(200);
					}
					while(Mix_Playing(3)) if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}
				}
				else if(EXPLODED) {
					Mix_PlayChannel(4, exploded_sound, 0);
					for(int i = 0; i < 14; i++) {
						if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
							close(); return 0;
						}

						SDL_Rect explodeSprite = {128 * (i & 3),128 * (i >> 2),128,128};

						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						game_background.render(0, 0);
						game_map.render(0, 0);

						for(int i = 0; i < MAX_SKULL; i++) {
							if(skull_curse[i]) skull[i].renderEntity();
							else skull[i].alert();
						}
						for(int i = 0; i < MAX_REAPER; i++) {
							if(reaper_curse[i]) reaper[i].renderEntity();
							else reaper[i].alert();
						}

						exploded_texture.render(player.entityPosition().x - ((exploded_texture.getWidth() - player.entityWidth()) >> 1), player.entityPosition().y - ((exploded_texture.getHeight() - player.entityHeight()) >> 1), &explodeSprite);

						showTime();

						SDL_RenderPresent(renderer);
						SDL_Delay(200);
					}
					while(Mix_Playing(4)) if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}
				}

				survival_time = survive_clock.getTick();

				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close();
						return 0;
					}

					dead_background.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.render(0, 0);
					game_map.render(0, 0);

					for(int i = 0; i < MAX_SKULL; i++) {
						if(skull_curse[i]) skull[i].renderEntity();
						else skull[i].alert();
					}
					for(int i = 0; i < MAX_REAPER; i++) {
						if(reaper_curse[i]) reaper[i].renderEntity();
						else reaper[i].alert();
					}

					showTime();

					dead_background.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close();
						return 0;
					}

					dead_background.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					dead_background.render(0, 0);

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
									if(player_name.length() > 0) player_name.pop_back();
									break;
								case SDLK_c:
									if(SDL_GetModState() && KMOD_CTRL) SDL_SetClipboardText(player_name.c_str());
									break;
								case SDLK_x:
									if(SDL_GetModState() && KMOD_CTRL) {
										SDL_SetClipboardText(player_name.c_str());
										player_name = "";
									}
									break;
								case SDLK_v:
									if(SDL_GetModState() && KMOD_CTRL) player_name += SDL_GetClipboardText();
									break;
								case SDLK_RETURN:
									confirmedName = true;
									break;
							}
							break;
						case SDL_TEXTINPUT:
							if(!(SDL_GetModState() && KMOD_CTRL && (e.text.text[0] == 'c' || e.text.text[0] == 'C' || e.text.text[0] == 'x' || e.text.text[0] == 'X' || e.text.text[0] == 'v' || e.text.text[0] == 'V'))) player_name += e.text.text;
							break;
					}
					if(!confirmedName) {
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
						SDL_RenderClear(renderer);

						if(player_name == "") player_name = " ";
						Texture nameTexture, instrucTexture;

						instrucTexture.loadFromRenderedText("Enter your name", deco_font, {255,0,0});
						instrucTexture.setSize(450, 100);
						instrucTexture.render((SCREEN_WIDTH - instrucTexture.getWidth()) >> 1, 210);

						nameTexture.loadFromRenderedText(player_name, deco_font, {91,44,111});
						nameTexture.setSize(30 * (int)player_name.length(), 100);
						nameTexture.render((SCREEN_WIDTH - nameTexture.getWidth()) >> 1, 330);

						SDL_RenderPresent(renderer);

						if(player_name == " ") player_name = "";
						nameTexture.free();
						instrucTexture.free();
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

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					if(player_name == "") player_name = " ";
					Texture textTexture, instrucTexture;

					instrucTexture.loadFromRenderedText("Enter your name", deco_font, {255,0,0});
					instrucTexture.setSize(450, 100);
					instrucTexture.render((SCREEN_WIDTH - instrucTexture.getWidth()) >> 1, 210);

					textTexture.loadFromRenderedText(player_name, deco_font, {91,44,111});
					textTexture.setSize(30 * (int)player_name.length(), 100);
					textTexture.render((SCREEN_WIDTH - textTexture.getWidth()) >> 1, 330);

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					if(player_name == " ") player_name = "";
					textTexture.free();
					instrucTexture.free();

					SDL_Delay(50);
				}
				SDL_Delay(500);

				play.setButtonColor(255, 255, 255);
				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					menu_background.render(0, 0);
					menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) >> 1, 25);

					play.renderButton();
					instruction.renderButton();
					high_score.renderButton();

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				option = MENU;
			}
			else {
				for(Uint8 frame = 0, alpha = 0; frame <= 17; frame++, alpha += 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					game_background.render(0, 0);
					game_map.render(0, 0);

					for(int i = 0; i < MAX_SKULL; i++) {
						if(skull_curse[i]) skull[i].renderEntity();
						else skull[i].alert();
					}
					for(int i = 0; i < MAX_REAPER; i++) {
						if(reaper_curse[i]) reaper[i].renderEntity();
						else reaper[i].alert();
					}
					player.renderEntity();

					showTime();

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);

				play.setButtonColor(255, 255, 255);
				for(Uint8 frame = 0, alpha = 255; frame <= 17; frame++, alpha -= 15) {
					if(SDL_PollEvent(&e) != 0 && e.type == SDL_QUIT) {
						close(); return 0;
					}

					transition_screen.setAlpha(alpha);

					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);

					menu_background.render(0, 0);
					menu_title.render((SCREEN_WIDTH - menu_title.getWidth()) >> 1, 25);

					play.renderButton();
					instruction.renderButton();
					high_score.renderButton();

					transition_screen.render(0, 0);

					SDL_RenderPresent(renderer);

					SDL_Delay(50);
				}
				SDL_Delay(500);
			}

			survive_clock.stop();
			break;

		//instruction part
		case INSTRUCTION:
			while(option == INSTRUCTION) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					back.handleEvent(&e);
				}
				switch(back.getCurrentSprite()) {
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
				if(option == INSTRUCTION) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);
					menu_background.render(0, 0);
					back.renderButton();
					SDL_RenderPresent(renderer);
					if(touch_back) {
						if(!lock_instruction_button_sound) {
							lock_instruction_button_sound = true;
							Mix_PlayChannel(1, touch_button, 0);
						}
					}
					else lock_instruction_button_sound = false;
				}
			}
			break;

		//high score part
		case HIGH_SCORE:
			while(option == HIGH_SCORE) {
				while(SDL_PollEvent(&e) != 0) {
					if(e.type == SDL_QUIT) {
						close(); return 0;
					}
					back.handleEvent(&e);
					resetScore.handleEvent(&e);
				}
				switch(back.getCurrentSprite()) {
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
				switch(resetScore.getCurrentSprite()) {
					case MOUSE_OUT:
						touch_reset = false;
						resetScore.setButtonColor(255, 255, 255);
						break;
					case MOUSE_IN:
						touch_reset = true;
						resetScore.setButtonColor(255, 0, 0);
						break;
					case MOUSE_DOWN:
						score_data.clear();
						score_data.resize(5);
						resetScore.reset();
						Mix_PlayChannel(1, click_button, 0);
						break;
				}
				if(option == HIGH_SCORE) {
					SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
					SDL_RenderClear(renderer);
					menu_background.render(0, 0);
					back.renderButton();
					resetScore.renderButton();
					for(int i = 0; i < score_data.size(); i++) {
						std::stringstream text;
						text.str("");
						text << (score_data[i].second != 0 ? score_data[i].first : "none") << std::string(5, ' ') << timeFormat(score_data[i].second);
						SDL_Color text_color;
						switch(i) {
							case 0: text_color = {255,215,0}; break;
							case 1:	text_color = {192,192,192}; break;
							default: text_color = {205, 127, 50}; break;
						}
						Texture data_information;
						data_information.loadFromRenderedText(text.str(), deco_font, text_color);
						data_information.setSize(20 * (int)text.str().length(), 80);
						data_information.render((SCREEN_WIDTH - data_information.getWidth()) >> 1, 120 + 80 * i);
						data_information.free();
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
			break;
	}
}