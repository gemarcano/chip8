#include "chip8.h"
#include "SDL_controller.h"

#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <thread>
#include <string>
#include <unordered_map>

template <typename T>
static std::vector<T> generate_sample(int len)
{       

	std::vector<T> dest(len);
	const size_t samples = len/sizeof(T);
	static size_t counter = 0;
	const size_t frequency = 1000; 
	const size_t period_in_samples = 48000/frequency;
	
	for (size_t i = 0; i < samples; ++i)
	{       
		int16_t sign = counter < period_in_samples/2 ? 1 : -1;
		dest[i] = sign * 28000;
		if (++counter >= period_in_samples)
		{       
			counter -= period_in_samples;
		}
	}

	return dest;
}

int main(int argc, char *argv[])
{
	using namespace chip8;
	using namespace std;
	emulator emu;
	unordered_map<SDL_Keycode, uint8_t> key_map;

	key_map[SDLK_1] = 1;
	key_map[SDLK_2] = 2;
	key_map[SDLK_3] = 3;
	key_map[SDLK_4] = 0xc;
	key_map[SDLK_q] = 4;
	key_map[SDLK_w] = 5;
	key_map[SDLK_e] = 6;
	key_map[SDLK_r] = 0xd;
	key_map[SDLK_a] = 7;
	key_map[SDLK_s] = 8;
	key_map[SDLK_d] = 9;
	key_map[SDLK_f] = 0xe;
	key_map[SDLK_z] = 0xa;
	key_map[SDLK_x] = 0;
	key_map[SDLK_c] = 0xb;
	key_map[SDLK_v] = 0xf;

	SDL_controller& sdl = SDL_controller::get_instance();
	sdl.init();
	
	std::string game_s("games/");
	if (argc >= 2)
	{
		game_s += argv[1];
	}
	else
	{
		game_s += "PONG";
	}
	ifstream game(game_s);
	if (game)
	{
		emu.load(game);
	}


	bool terminate = false;

	while (!terminate)
	{
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				terminate = true;
				break;
			case SDL_KEYDOWN:
				if (key_map.count(event.key.keysym.sym))
				{
					emu.setKey(key_map[event.key.keysym.sym]);
				}
				break;
			case SDL_KEYUP:
				if (key_map.count(event.key.keysym.sym))
				{
					emu.unsetKey(key_map[event.key.keysym.sym]);
				}
			}
		}
		
		emu.run(std::chrono::duration<double, std::milli>(1000./60));
		
		sdl.draw(emu.getGfx().data());

		if (emu.getSound())
		{
			sdl.play_sound(generate_sample<int16_t>, 100ms);
		}

		std::chrono::duration<double, std::milli> hz60(100./6.);
		std::this_thread::sleep_until(now + hz60);
		sdl.present();
	}

	return 0;
}
