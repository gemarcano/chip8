#ifndef _CHIP8_SPC_CONTROLLER_H_
#define _CHIP8_SPC_CONTROLLER_H_

#include <SDL2/SDL.h>

#include <chrono>
#include <functional>
#include <vector>

namespace chip8
{
	class SDL_controller
	{
	public:
		~SDL_controller();
		static SDL_controller& get_instance();

		bool init();

		void clear();
		void draw(const bool *aCont);
		void present();

		template<typename Func, class Rep, class Period>
		void play_sound(Func aFunctor, const std::chrono::duration<Rep, Period>& aTime);

		void beep();
		void unbeep();

	private:
		static SDL_controller mInst;
		SDL_controller();
		SDL_controller(SDL_controller&&) = default;
		SDL_controller(SDL_controller&) = delete;
		SDL_controller& operator=(SDL_controller&) = delete;
		SDL_controller& operator=(SDL_controller&&) = default;

		bool mInit;
		bool mError;

		SDL_Window *mpWin;
		SDL_Renderer *mpRen;
		SDL_AudioSpec mWanted;
		SDL_AudioDeviceID mDev;
		std::function<std::vector<int16_t>(int)> mGenerate_samples;
		static void mSound_Callback(void * udata, Uint8 *stream, int len);
		int16_t mSound_buffer[2046];
		std::chrono::steady_clock::time_point mEnd;
	};

	template<typename Func, class Rep, class Period>
	void SDL_controller::play_sound(Func aFunctor, const std::chrono::duration<Rep, Period>& aTime)
	{
		//setup callback to use generator
		//FIXME do we need locking for concurrency?
		
		mEnd = std::chrono::steady_clock::now() + aTime;
		mGenerate_samples = std::function<std::vector<int16_t>(int)>(aFunctor);
		SDL_PauseAudioDevice(mDev, 0);
	}


}

#endif//_CHIP8_SPC_CONTROLLER_H_
