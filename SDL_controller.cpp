#include "SDL_controller.h"

#include <SDL2/SDL.h>
#include <array>

namespace chip8
{
	SDL_controller SDL_controller::mInst;

	template <typename T, size_t Size>
	void generate_sample(std::array<T,Size>& dest, size_t len)
	{
		size_t samples = len/sizeof(T);
		static size_t counter = 0;
		size_t frequency = 1000;
		size_t period_in_samples = 48000/frequency;

		for (size_t i = 0; i < samples; ++i)
		{
			int16_t sign = counter < period_in_samples/2 ? 1 : -1;
			dest[i] = sign * 28000;
			if (++counter >= period_in_samples)
			{
				counter -= period_in_samples;
			}
		}
	}

	void fill_audio(void *, Uint8 *stream, int len)
	{               
		std::array<int16_t, 2048> s_buffer;
		generate_sample(s_buffer, len);
		memcpy(stream, s_buffer.data(), len);
	}       
		
	SDL_controller::SDL_controller()
	:mInit(false), mError(false), mpWin(nullptr), mpRen(nullptr)
	{       
		mWanted.freq = 48000;
		mWanted.format = AUDIO_S16SYS;
		mWanted.channels = 1;
		mWanted.samples = 2048;
		mWanted.callback = fill_audio;
		mWanted.userdata = this;
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

			mDev = SDL_OpenAudioDevice(NULL, 0, &mWanted, NULL, 0);
			if (!mDev)
			{
				mError = true;
				mInit = false;
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
			SDL_PauseAudioDevice(mDev, 1);
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

}
