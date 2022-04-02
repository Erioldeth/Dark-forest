#ifndef RAND_MACHINE
#define RAND_MACHINE
#include<SDL.h>
#include<random>
#include"const_value.h"
std::random_device rd;
std::mt19937 mt(rd());

std::uniform_int_distribution<> curse_creation(1, 1024);
bool skull_creation() {
	return !(curse_creation(mt) & 511);
}
bool reaper_creation() {
	return curse_creation(mt) == 1024;
}

std::uniform_int_distribution<> play_sound(1, 128);

std::uniform_int_distribution<> particle_life_init(MAX_PARTICLE_LIFE >> 1, MAX_PARTICLE_LIFE);
#endif // !RAND_MACHINE
