#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <cstdint>
#include <array>
#include <istream>
#include <chrono>

namespace chip8
{
	typedef std::uint16_t opcode_t;
	typedef std::uint16_t memory_t; 

	class emulator
	{
	public:
		constexpr emulator()
		:frequency(1000), memory{}, registers{{0},0,0x200}, gfx{0}, 
			delay_timer{0}, delay_counter{0}, sound_timer{0},
			sound_counter{0}, sound{false}, stack{0}, sp{0}, key{0}
		{};
		
		~emulator() = default;
		emulator(emulator&&) = default;
		emulator(emulator&) = delete;
		emulator& operator=(emulator&) = delete;
		emulator& operator=(emulator&&) = default;

		void load(std::istream& aGame);
		void run(size_t aInstructions);
		
		template <class rep, class ratio>
		void run(const std::chrono::duration<rep, ratio>& aDuration)
		{
			size_t count = aDuration.count() * frequency *
				ratio::num / ratio::den;
			run(count);
		}

		constexpr const std::array<bool, 64 * 32> getGfx() const
		{
			return gfx;
		}

		void setKey(std::uint8_t aKey)
		{
			key[aKey & 0xFF] = 1;
		}

		void unsetKey(std::uint8_t aKey)
		{
			key[aKey & 0xFF] = 0;
		}

		bool getSound()
		{
			bool result = sound;
			sound = false;
			return result;
		}

	private:

		unsigned int frequency;

		class RAM
		{
		public:
			constexpr RAM(): memory{} {};
			std::uint8_t& operator[](memory_t aAddress);
			constexpr const std::uint8_t& operator[](memory_t aAddress) const;
			constexpr uint16_t get2Bytes(memory_t aAddress) const;
		private:
			std::array<std::uint8_t, 0x1000> memory;
		} memory;

		struct
		{
			std::array<std::uint8_t, 16> V;
			std::uint16_t I;
			std::uint16_t pc;

			void increment_pc() { pc = (pc + 2) & 0xFFF ; }
		} registers;

		std::array<bool, 64 * 32> gfx;
	
		std::uint8_t delay_timer;
		unsigned int delay_counter;
		std::uint8_t sound_timer;
		unsigned int sound_counter;
		bool sound;

		std::array<memory_t, 16> stack;
		memory_t sp;
		
		void stack_push();
		memory_t stack_pop();
		
		std::array<std::uint8_t, 16> key;

		std::uint8_t& getV(memory_t, std::uint8_t);
		typedef void (emulator::*operation)(opcode_t aOpcode);
		
		opcode_t fetch();
		operation decode(opcode_t aOp);
		void execute(operation aOperation, opcode_t aOpcode);
		void timers();

		static const std::uint8_t chip8_fontset[80];

		//Opcode implementations
		void RCA(opcode_t);
		void clear_screen(opcode_t);
		void return_sub(opcode_t);
		void jump(opcode_t aOp);
		void subr(opcode_t aOp);
		void skip_equal(opcode_t aOp);
		void skip_nequal(opcode_t aOp);
		void skip_vequal(opcode_t aOp);
		void set_v(opcode_t aOp);
		void add_v(opcode_t aOp);
		void set_vxy(opcode_t aOp);
		void set_vxy_or(opcode_t aOp);
		void set_vxy_and(opcode_t aOp);
		void set_vxy_xor(opcode_t aOp);
		void add_vxy(opcode_t aOp);
		void sub_vxy(opcode_t aOp);
		void shiftr_vx(opcode_t aOp);
		void sub_vyx(opcode_t aOp);
		void shiftl_vx(opcode_t aOp);
		void skip_vnequal(opcode_t aOp);
		void set_i(opcode_t aOp);
		void jump_v0(opcode_t aOp);
		void random(opcode_t aOp);
		void draw(opcode_t aOp);
		void skip_keyx(opcode_t aOp);
		void skip_nkeyx(opcode_t aOp);
		void delay_to_vx(opcode_t aOp);
		void wait_key(opcode_t aOp);
		void set_delay(opcode_t aOp);
		void set_sound(opcode_t aOp);
		void add_ivx(opcode_t aOp);
		void set_sprite(opcode_t aOp);
		void BCD(opcode_t aOp);
		void save_r(opcode_t aOp);
		void load_r(opcode_t aOp); 
	};
}

#endif//_CHIP8_H_
