#include "chip8.h"

#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include <thread>
#include <string>

#include <unordered_map>

class SDL_controller
{
public:
	~SDL_controller();
	static SDL_controller& get_instance();

	bool init();

	void clear();
	void draw(const bool *aCont);
	void present();

	void beep();
	void unbeep();

private:
	static SDL_controller mInst;
	SDL_controller();
	SDL_controller(SDL_controller&&) = default;
	SDL_controller(SDL_controller&) = delete;
	SDL_controller& operator=(SDL_controller&) = delete;
	SDL_controller& operator=(SDL_controller&&) = default;

	static void fill_audio(void *udata, Uint8 *stream, int len);

	bool mInit;
	bool mError;

	SDL_Window *mpWin;
	SDL_Renderer *mpRen;
	SDL_AudioSpec mWanted;
	SDL_AudioDeviceID mDev;
};

SDL_controller SDL_controller::mInst;

int16_t sound_buffer[1024];

SDL_controller::SDL_controller()
:mInit(false), mError(false), mpWin(nullptr), mpRen(nullptr)
{
	mWanted.freq = 48000;
	mWanted.format = AUDIO_S16SYS;
	mWanted.channels = 1;
	mWanted.samples = 1024;
	mWanted.callback = fill_audio;
	mWanted.userdata = this;
}

void SDL_controller::fill_audio(void *, Uint8 *stream, int len)
{
	memcpy(stream, &sound_buffer, len);	
}

bool SDL_controller::init()
{
	if (!mInit)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
		{
			mError = true;
		}
		else
		{
			mInit = true;
		}
	}

	if (mInit)
	{
		mpWin = SDL_CreateWindow("Chip8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, SDL_WINDOW_SHOWN);
		if (!mpWin)
		{
			mError = true;
			mInit = false;
		}

		mpRen = SDL_CreateRenderer(mpWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (!mpRen)
		{
			mError = true;
			mInit = false;
		}

		SDL_RenderSetLogicalSize(mpRen, 64, 32);

		SDL_SetRenderDrawColor(mpRen, 0, 0, 0, 1);
		SDL_RenderClear(mpRen);
		
		SDL_RenderPresent(mpRen);

		if (!(mDev = SDL_OpenAudioDevice(NULL, 0, &mWanted, NULL, 0)))
		{
			printf("SDL_Init failed: %s\n", SDL_GetError());
			mError = true;
			mInit = false;
		}

		for (size_t i = 0; i < sizeof(sound_buffer)/sizeof(*sound_buffer); ++i)
		{
			if ( i % (48000/440) < (48000/440/2))
			{
				sound_buffer[i] = 28000;
			}
			else
			{
				sound_buffer[i] = -28000;
			}
		}
	}
	return !mError;
}

void SDL_controller::draw(const bool *array)
{
	for (unsigned char j = 0; j < 32; ++j)
	{
		for (unsigned char i = 0; i < 64; ++i)
		{
			if (array[i + j*64])
				SDL_RenderDrawPoint(mpRen, i, j);
		}
	}
}

void SDL_controller::present()
{
	SDL_RenderPresent(mpRen);

	SDL_SetRenderDrawColor(mpRen, 0, 0, 0, 1);
	SDL_RenderClear(mpRen);

	SDL_SetRenderDrawColor(mpRen, 0, 150, 0, 1);
}

void SDL_controller::beep()
{	
	SDL_PauseAudioDevice(mDev, 0);
}

void SDL_controller::unbeep()
{
	SDL_PauseAudioDevice(mDev, 1);
}

SDL_controller::~SDL_controller()
{

	if (mDev)
	{
		SDL_CloseAudioDevice(mDev);
	}

	if (mpRen)
	{
		SDL_DestroyRenderer(mpRen);
		mpRen = nullptr;
	}

	if (mpWin)
	{
		SDL_DestroyWindow(mpWin);
		mpWin = nullptr;
	}
	
	SDL_Quit();
}

SDL_controller& SDL_controller::get_instance()
{
	return mInst;
}

#define FREQ 1000

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
			sdl.beep();
		}
		else
		{
			sdl.unbeep();
		}

		std::chrono::duration<double, std::milli> hz60(100./6.);
		std::this_thread::sleep_until(now + hz60);
		sdl.present();
	}

	return 0;
}
