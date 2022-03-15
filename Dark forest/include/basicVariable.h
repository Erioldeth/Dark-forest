//#pragma once
#ifndef BASIC_VAR
#define BASIC_VAR
#include<SDL.h>
#include<SDL_image.h>
#include<SDL_ttf.h>
#include<SDL_mixer.h>
#include<iostream>
#include<vector>
#include<string>
#include<tuple>
#include"const_value.h"
#include"timer.h"

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

//shared items
enum gameOption {
	MENU,
	PLAY,
	INSTRUCTION,
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
bool touch_play = false, touch_instruction = false, touch_high_score = false, lock_menu_button_sound = false;

//game play
Timer survive_clock;
bool EXPLODED = false, SHADOW_CAUGHT = false, DEAD = false, PAUSED = false;
std::string player_name;
Uint32 survival_time = 0;
std::vector<SDL_Rect> platform(16);
int identification = 0;
std::vector<bool> skull_curse(MAX_SKULL), reaper_curse(MAX_REAPER);
Mix_Chunk* exploded_sound = nullptr;
Mix_Chunk* shadow_caught_sound = nullptr;
bool touch_pause = false, lock_pause_button_sound = false, touch_resume = false, touch_exit = false, lock_resume_and_exit_button_sound = false;

//instruction
bool lock_instruction_button_sound = false;

//high score
std::vector<std::pair<std::string, Uint32>> score_data;
bool touch_reset = false, lock_high_score_button_sound = false;
#endif // !BASIC_VAR
