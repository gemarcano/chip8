#ifndef _CHIP8_SPC_CONTROLLER_H_
#define _CHIP8_SPC_CONTROLLER_H_

#include <SDL2/SDL.h>

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
	};

}

#endif//_CHIP8_SPC_CONTROLLER_H_
