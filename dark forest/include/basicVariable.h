//#pragma once
#ifndef BASIC_VAR
#define BASIC_VAR
#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<fstream>
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
bool touch_back = false;


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
int music_volume;
int touch_button_volume, click_button_volume;
int player_move_volume;
int skuLL_move_volume, exploded_volume;
int reaper_move_volume, reaper_die_volume, shadow_caught_volume;

//high score
std::vector<std::tuple<std::string, Uint8, Uint32>> score_data;
bool touch_reset = false, lock_high_score_button_sound = false;
#endif // !BASIC_VAR
