#include "chip8.h"

#include <cstdint>
#include <array>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <random>

namespace chip8
{
	template <class T, class T2>
	std::uint8_t getNibble(T aNum, T2 aNibble)
	{
		return (aNum >> (aNibble * 4)) & 0xF;
	}

	std::uint8_t& emulator::getV(memory_t aOp, std::uint8_t aNibble)
	{
		return registers.V[getNibble(aOp, aNibble)];
	}

	#define NX getNibble(aOp, 2)
	#define NY getNibble(aOp, 1)

	#define VX getV(aOp, 2)
	#define VY getV(aOp, 1)

	#define FONT_ADDR 0x50

	const std::uint8_t emulator::chip8_fontset[80] = 
	{ 
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80	// F
	};


	void emulator::load(std::istream& aGame)
	{
		memcpy(&memory[0] + FONT_ADDR, chip8_fontset, sizeof(chip8_fontset));

		for (memory_t i = 0; aGame && aGame.peek() != std::istream::traits_type::eof(); ++i)
		{
			memory[0x200 + i] = static_cast<unsigned char>(aGame.get());
		}
	}

	void emulator::run(size_t aInstructions)
	{
		for (size_t i = 0; i < aInstructions; ++i)
		{
			opcode_t op = fetch();
			operation oper = decode(op);
			execute(oper, op);
			timers();
		}
	}	

	constexpr const std::uint8_t& emulator::RAM::operator[](memory_t aAddress) const
	{
		return memory[aAddress & 0x0FFF];
	}

	std::uint8_t& emulator::RAM::operator[](memory_t aAddress)
	{
		return const_cast<std::uint8_t&>(static_cast<const emulator::RAM&>(*this)[aAddress]);
	}

	constexpr std::uint16_t emulator::RAM::get2Bytes(memory_t aAddress) const
	{
		std::uint16_t result = memory[aAddress & 0x0FFF] << 8;
		result |= memory[(aAddress+1) & 0xFFF];
		return result;
	}

	opcode_t emulator::fetch()
	{
		return memory.get2Bytes(registers.pc);
	}


	emulator::operation emulator::decode(opcode_t aOp)
	{
		operation result = nullptr;
		switch (aOp & 0xF000)
		{
			case 0x0000:
			switch (aOp)
			{
				case 0x00E0:
					result = &emulator::clear_screen;
				break;
				case 0x00EE:
					result = &emulator::return_sub;
				break;
				default:
					result = &emulator::RCA;
				
			}
			break;

			case 0x1000:
				result = &emulator::jump;
			break;
			case 0x2000:
				result = &emulator::subr;
			break;
			case 0x3000:
				result = &emulator::skip_equal;
			break;
			case 0x4000:
				result = &emulator::skip_nequal;
			break;
			case 0x5000:
				result = &emulator::skip_vequal;
			break;
			case 0x6000:
				result = &emulator::set_v;
			break;		
			case 0x7000:
				result = &emulator::add_v;
			break;
			case 0x8000:
			switch (aOp & 0xF)
			{
				case 0x0:
					result = &emulator::set_vxy;
				break;
				case 0x1:
					result = &emulator::set_vxy_or;
				break;
				case 0x2:
					result = &emulator::set_vxy_and;
				break;
				case 0x3:
					result = &emulator::set_vxy_xor;
				break;
				case 0x4:
					result = &emulator::add_vxy;
				break;
				case 0x5:
					result = &emulator::sub_vxy;
				break;
				case 0x6:
					result = &emulator::shiftr_vx;
				break;
				case 0x7:
					result = &emulator::sub_vyx;
				break;
				case 0xE:
					result = &emulator::shiftl_vx;
				break;
			}
			break;
			case 0x9000:
				result = &emulator::skip_vnequal;
			break;		
			case 0xA000:
				result = &emulator::set_i;
			break;
			case 0xB000:
				result = &emulator::jump_v0;
			break;
			case 0xC000:
				result = &emulator::random;
			break;
			case 0xD000:
				result = &emulator::draw;
			break;
			case 0xE000:
			switch (aOp & 0xFF)
			{
				case 0x9E:
					result = &emulator::skip_keyx;
				break;
				case 0xA1:
					result = &emulator::skip_nkeyx;
				break;
			}
			break;
			case 0xF000:
			switch (aOp & 0xFF)
			{
				case 0x07:
					result = &emulator::delay_to_vx;
				break;
				case 0x0A:
					result = &emulator::skip_nkeyx;
				break;
				case 0x15:
					result = &emulator::set_delay;
				break;
				case 0x18:
					result = &emulator::set_sound;
				break;
				case 0x1E:
					result = &emulator::add_ivx;
				break;
				case 0x29:
					result = &emulator::set_sprite;
				break;
				case 0x33:
					result = &emulator::BCD;
				break;
				case 0x55:
					result = &emulator::save_r;
				break;
				case 0x65:
					result = &emulator::load_r;
				break;
			}
		}
		return result;
	}

	void emulator::timers()
	{
		
		hertz60_counter++;
		if (frequency/60. > hertz60_counter)
		{
			hertz60_counter -= frequency/60.;
			hertz60_clock++;
		}

		if (delay_timer)
		{
			delay_counter++;
			if (frequency/60. > delay_counter)
			{
				delay_counter -= frequency/60.;
				delay_timer--;
			}
		}
		else if (delay_counter)
		{
			delay_counter = 0;
		}

		if (sound_timer)
		{
			sound_counter++;
			if (frequency/60. > sound_counter)
			{
				sound_counter -= frequency/60.;
				sound_timer--;

				if (!sound_timer)
				{
					sound = true;
					sound_timestamp = hertz60_clock;
				}
			}
		}
		else if (sound_counter)
		{
			sound_counter = 0;
		}

		if (sound)
		{
			if (sound_timestamp + 10 < hertz60_clock)
			{
				sound = false;
			}
		}
	}

	void emulator::stack_push()
	{
		stack[sp] = registers.pc;
		sp = sp < stack.size() ? sp+1 : sp;
	}

	memory_t emulator::stack_pop()
	{
		if (sp)
			return stack[--sp];
		return 0;
	}

	void emulator::execute(operation aOperation, opcode_t aOpcode)
	{
		(this->*aOperation)(aOpcode);
		registers.increment_pc();
	}

	void emulator::RCA(opcode_t)
	{
		
	}

	void emulator::clear_screen(opcode_t)
	{
		gfx = {};
	}

	void emulator::return_sub(opcode_t)
	{
		registers.pc = stack_pop();
	}

	void emulator::jump(opcode_t aOp)
	{
		registers.pc = (aOp - 2) & 0x0FFF;
	}

	void emulator::subr(opcode_t aOp)
	{
		stack_push();
		registers.pc = (aOp - 2) & 0x0FFF;
	}

	void emulator::skip_equal(opcode_t aOp)
	{
		if (VX == (aOp & 0xFF))
			registers.increment_pc();
	}

	void emulator::skip_nequal(opcode_t aOp)
	{
		if (VX != (aOp & 0xFF))
			registers.increment_pc();
	}

	void emulator::skip_vequal(opcode_t aOp)
	{
		if (VX != VY)
			registers.increment_pc();
	}

	void emulator::set_v(opcode_t aOp)
	{
		VX = aOp & 0xFF;
	}

	void emulator::add_v(opcode_t aOp)
	{
		VX += aOp & 0xFF;
	}

	void emulator::set_vxy(opcode_t aOp)
	{
		VX = VY;
	}

	void emulator::set_vxy_or(opcode_t aOp)
	{
		VX |= VY;
	}

	void emulator::set_vxy_and(opcode_t aOp)
	{
		VX &= VY;
	}

	void emulator::set_vxy_xor(opcode_t aOp)
	{
		VX ^= VY;
	}

	void emulator::add_vxy(opcode_t aOp)
	{
		uint16_t result = VX + VY;
		VX = result & 0xFF;
		registers.V.back() = !!(result & 0xF00);
	}

	void emulator::sub_vxy(opcode_t aOp)
	{
		registers.V.back() = (VX > VY);
		VX -= VY;	
	}

	void emulator::shiftr_vx(opcode_t aOp)
	{
		registers.V.back() = VX & 0x1;
		VX >>= 1;
	}

	void emulator::sub_vyx(opcode_t aOp)
	{
		registers.V.back() = (VY > VX);
		VX = VY - VX;	
	}

	void emulator::shiftl_vx(opcode_t aOp)
	{
		registers.V.back() = !!(VX & 0x80);
		VX <<= 1;
	}

	void emulator::skip_vnequal(opcode_t aOp)
	{	
		if (VY != VX)
			registers.increment_pc();
	}

	void emulator::set_i(opcode_t aOp)
	{
		registers.I = aOp & 0xFFF;
	}

	void emulator::jump_v0(opcode_t aOp)
	{
		registers.pc = (aOp & 0xFFF) + registers.V[0] - 2;
	}

	void emulator::random(opcode_t aOp)
	{
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<unsigned short> dist(0, 255);
		VX = static_cast<std::uint8_t>(dist(mt)) & (aOp & 0xFF);
	}

	void emulator::draw(opcode_t aOp)
	{
		registers.V.back() = 0;
		for (std::uint8_t j = 0; j < (aOp & 0xF); ++j)
		{
			size_t pos;
			bool cur_bit;
			bool mem_bit;
			for (std::uint8_t i = 0; i < 8; ++i)
			{
				pos = (VX + i) % 64 + ((VY + j) % 32)* 64;
				cur_bit = gfx[pos];
				mem_bit = !!(memory[registers.I + j] & (0x80 >> i));
				gfx[pos] = cur_bit != mem_bit;
				if (cur_bit && !gfx[pos])
					registers.V.back() = 1;
			}
		}
	}

	void emulator::skip_keyx(opcode_t aOp)
	{
		if (VX < 16 && key[VX])
			registers.increment_pc();
	}

	void emulator::skip_nkeyx(opcode_t aOp)
	{
		if (VX >= 16 || !key[VX])
			registers.increment_pc();
	}

	void emulator::delay_to_vx(opcode_t aOp)
	{
		VX = delay_timer;
	}

	void emulator::wait_key(opcode_t aOp)
	{
		for (std::uint8_t i = 0; i < 16; ++i)
		{
			if (key[i])
			{
				VX = i;
				return;
			}
		}
		registers.pc -= 2; //Decrement PC since it'll be incremented in exec-- repeat cycle
	}

	void emulator::set_delay(opcode_t aOp)
	{
		delay_timer = VX;
	}

	void emulator::set_sound(opcode_t aOp)
	{
		sound_timer = VX;
	}

	void emulator::add_ivx(opcode_t aOp)
	{
		registers.I += VX;
	}

	void emulator::set_sprite(opcode_t aOp)
	{
		unsigned char index = VX & 0xF;
		registers.I = FONT_ADDR + index * 5;
	}

	void emulator::BCD(opcode_t aOp)
	{
		const std::uint8_t& num = VX;
		std::uint8_t* result[3] = {&memory[registers.I + 2], &memory[registers.I+1], &memory[registers.I] };
		*result[0] = *result[1] = *result[2] = 0;
		for (std::uint8_t i = 0; i < 8; ++i)
		{
			for (std::uint8_t j = 0; j < 3; ++j)
			{
				if (*result[j] >= 5)
					*result[j] += 3;
			}
			*result[2] <<= 1;
			*result[2] |= !!(*result[1] & 0x08);
			*result[1] <<= 1;
			*result[1] |= !!(*result[0] & 0x08);
			*result[0] <<= 1;
			*result[0] |= !!(num & (1 << (7-i)));
			*result[2] &= 0xF;
			*result[1] &= 0xF;
			*result[0] &= 0xF;
		}
	}

	void emulator::save_r(opcode_t aOp)
	{
		for (std::uint8_t i = 0; i < std::min<size_t>(registers.V.size(), NX + 1); ++i)
		{
			memory[registers.I + i] = registers.V[i];
		}
	}

	void emulator::load_r(opcode_t aOp)
	{
		for (std::uint8_t i = 0; i < std::min<size_t>(registers.V.size(), NX + 1); ++i)
		{
			registers.V[i] = memory[registers.I + i];
		}
	}
}

