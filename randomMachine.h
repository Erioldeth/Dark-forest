#pragma once
#ifndef RAND_MACHINE
#define RAND_MACHINE
#include<SDL.h>
#include<random>
std::random_device rd;
std::mt19937 mt(rd());

std::uniform_int_distribution<> curse_construction(1, 1024);
std::uniform_int_distribution<> sound_play(1, 128);

bool skull_curse_construction() {
	Uint16 curse_creation = curse_construction(mt);
	if((curse_creation & 511) == 0) return true;
	return false;
}
bool reaper_curse_construction() {
	Uint16 curse_creation = curse_construction(mt);
	if(curse_creation == 1024) return true;
	return false;
}
#endif // !RAND_MACHINE
