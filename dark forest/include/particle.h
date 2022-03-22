#ifndef PARTICLE_H
#define PARTICLE_H
#include"randomMachine.h"
#include"texture.h"
class Particle {
	Texture particle_texture;
	std::vector<std::tuple<SDL_Point, SDL_Rect, int>> particle_info;
public:
	Particle() {
		particle_info.resize(MAX_PARTICLE);
	}
	void loadParticleTexture(std::string path, Uint8 r = 1, Uint8 g = 1, Uint8 b = 1) {
		particle_texture.loadTextureFromFile(path, r, g, b);
		particle_texture.setTextureBlendMode(SDL_BLENDMODE_BLEND);
	}
	void setParticleSize(int w, int h) {
		particle_texture.setTextureSize(w, h);
	}
	void setParticleColor(Uint8 r, Uint8 g, Uint8 b) {
		particle_texture.setTextureColor(r, g, b);
	}
	void renderParticle(SDL_Rect& region, int totalSprite, int col, int original_width, int original_height) {
		for(auto& p : particle_info) {
			if(get<2>(p) == 0) {
				std::uniform_int_distribution<> horizontal_change(-5, region.w + 5 - particle_texture.textureWidth());
				std::uniform_int_distribution<> vertical_change(-5, region.h + 5 - particle_texture.textureHeight());
				get<0>(p) = {region.x + horizontal_change(mt),region.y + vertical_change(mt)};

				std::uniform_int_distribution<> rand_sprite(0, totalSprite - 1);
				int frame = rand_sprite(mt);
				get<1>(p) = {original_width * (frame % col),original_height * (frame / col),original_width,original_height};

				get<2>(p) = particle_life_init(mt);
			}
			short a = get<2>(p) * 255 / MAX_PARTICLE_LIFE;
			particle_texture.setTextureAlpha((Uint8)a);
			particle_texture.renderTexture(get<0>(p).x, get<0>(p).y, &get<1>(p));
			get<2>(p)--;
		}
	}
	void reset() {
		for(auto& p : particle_info) get<2>(p) = 0;
	}
	void free() {
		particle_texture.free();
		particle_info.clear();
	}
};
#endif // !PARTICLE_H
