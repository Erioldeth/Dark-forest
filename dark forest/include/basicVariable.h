#ifndef BASIC_VAR_H
#define BASIC_VAR_H
#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<fstream>
#include<cmath>
#include<array>
#include<vector>
#include<string>
#include<tuple>
#include"const_value.h"
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;



//shared items
enum gameOption {
	MENU,
	PLAY,
	SETTING,
	HIGH_SCORE
};
Uint8 option = MENU;
TTF_Font* button_font = nullptr;
TTF_Font* deco_font = nullptr;
Mix_Chunk* touch_button = nullptr;
Mix_Chunk* click_button = nullptr;
bool touch_back = false, touch_reset = false;



//menu
Mix_Music* menu_music = nullptr;
bool touch_play = false, touch_setting = false, touch_high_score = false, lock_menu_button_sound = false;



//game play
enum gameMode {
	NONE,
	EASY,
	NORMAL,
	HARD
};
Uint8 mode = NONE;
bool touch_easy = false, touch_normal = false, touch_hard = false, lock_mode_button_sound = false;

int CURRENT_SKULL = MAX_SKULL, CURRENT_REAPER = MAX_REAPER;
bool EXPLODED = false, SHADOW_CAUGHT = false, DEAD = false, PAUSED = false;

std::string player_name;
Uint32 survival_time = 0;
std::vector<SDL_Rect> platform(16);

int identification = 0;
std::vector<bool> skull_curse(MAX_SKULL), reaper_curse(MAX_REAPER);
Mix_Chunk* exploded_sound = nullptr;
Mix_Chunk* shadow_caught_sound = nullptr;

bool touch_pause = false, lock_pause_button_sound = false, touch_resume = false, touch_exit = false, lock_resume_and_exit_button_sound = false;



//setting
bool lock_setting_button_sound = false;
std::vector<int> volume_ratio(2);
std::vector<std::vector<Uint8>> color_code(2, std::vector<Uint8>(3));
std::vector<bool> touch_bar(2), touch_sample(2), picker_activated(2), touch_picker(2);
SDL_Surface* picker_surface = nullptr;
int model_movement = 0;



//high score
std::vector<std::tuple<std::string, Uint8, Uint32>> score_data;
bool lock_high_score_button_sound = false;
#endif // !BASIC_VAR_H
