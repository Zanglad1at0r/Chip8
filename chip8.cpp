#include <iostream>
#include <fstream>
#include <cstdint>
#include <random>
#include <chrono>
#include <cstring>
#include <iomanip>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int SCREEN_WIDTH = 64;
const unsigned int SCREEN_HEIGHT = 32;

class Chip8
{
public:
	uint8_t registers[16];
	uint8_t memory[4096];
	uint16_t pc;
	uint16_t index;
	uint16_t stack[16];
	uint8_t sp;
	uint8_t delayTimer;
	uint8_t soundTimer;
	uint8_t keypad[16];
	uint32_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
	uint16_t opcode;
	uint8_t fontset[FONTSET_SIZE] = {
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
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	Chip8()
		: randGen(std::chrono::system_clock::now().time_since_epoch().count())
	{
		// init pc
		pc = START_ADDRESS;

		// load fontset into ROM from 0x50 to 0x9F
		for (int i = 0; i < FONTSET_SIZE; i++)
		{
			memory[FONTSET_START_ADDRESS + i] = fontset[i];
		}

		randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

		table[0x0] = &Chip8::Table0;
		table[0x1] = &Chip8::OP_1nnn;
		table[0x2] = &Chip8::OP_2nnn;
		table[0x3] = &Chip8::OP_3xkk;
		table[0x4] = &Chip8::OP_4xkk;
		table[0x5] = &Chip8::OP_5xy0;
		table[0x6] = &Chip8::OP_6xkk;
		table[0x7] = &Chip8::OP_7xkk;
		table[0x8] = &Chip8::Table8;
		table[0x9] = &Chip8::OP_9xy0;
		table[0xA] = &Chip8::OP_Annn;
		table[0xB] = &Chip8::OP_Bnnn;
		table[0xC] = &Chip8::OP_Cxkk;
		table[0xD] = &Chip8::OP_Dxyn;
		table[0xE] = &Chip8::TableE;
		table[0xF] = &Chip8::TableF;

		for (size_t i = 0; i <= 0xE; i++)
		{
			table0[i] = &Chip8::OP_NULL;
			table8[i] = &Chip8::OP_NULL;
			tableE[i] = &Chip8::OP_NULL;
		}

		table0[0x0] = &Chip8::OP_00E0;
		table0[0xE] = &Chip8::OP_00EE;

		table8[0x0] = &Chip8::OP_8xy0;
		table8[0x1] = &Chip8::OP_8xy1;
		table8[0x2] = &Chip8::OP_8xy2;
		table8[0x3] = &Chip8::OP_8xy3;
		table8[0x4] = &Chip8::OP_8xy4;
		table8[0x5] = &Chip8::OP_8xy5;
		table8[0x6] = &Chip8::OP_8xy6;
		table8[0x7] = &Chip8::OP_8xy7;
		table8[0xE] = &Chip8::OP_8xyE;

		tableE[0x1] = &Chip8::OP_ExA1;
		tableE[0xE] = &Chip8::OP_Ex9E;

		for (size_t i = 0; i <= 0x65; i++)
		{
			tableF[i] = &Chip8::OP_NULL;
		}

		tableF[0x07] = &Chip8::OP_Fx07;
		tableF[0x0A] = &Chip8::OP_Fx0A;
		tableF[0x15] = &Chip8::OP_Fx15;
		tableF[0x18] = &Chip8::OP_Fx18;
		tableF[0x1E] = &Chip8::OP_Fx1E;
		tableF[0x29] = &Chip8::OP_Fx29;
		tableF[0x33] = &Chip8::OP_Fx33;
		tableF[0x55] = &Chip8::OP_Fx55;
		tableF[0x65] = &Chip8::OP_Fx65;
	};

	void Table0()
	{
		((*this).*(table0[opcode & 0x000Fu]))();
	}

	void Table8()
	{
		((*this).*(table8[opcode & 0x000Fu]))();
	}

	void TableE()
	{
		((*this).*(tableE[opcode & 0x000Fu]))();
	}

	void TableF()
	{
		((*this).*(tableF[opcode & 0x00FFu]))();
	}

	typedef void (Chip8::*Chip8Func)();
	Chip8Func table[0xF + 1];
	Chip8Func table0[0xE + 1];
	Chip8Func table8[0xE + 1];
	Chip8Func tableE[0xE + 1];
	Chip8Func tableF[0x65 + 1];

	std::default_random_engine randGen;
	std::uniform_int_distribution<uint8_t> randByte;

	void LoadROM(char const *filename);
	void Cycle();
	void logOP();
	void OP_NULL() {}
	void OP_00E0();
	void OP_00EE();
	void OP_1nnn();
	void OP_2nnn();
	void OP_3xkk();
	void OP_4xkk();
	void OP_5xy0();
	void OP_6xkk();
	void OP_7xkk();
	void OP_8xy0();
	void OP_8xy1();
	void OP_8xy2();
	void OP_8xy3();
	void OP_8xy4();
	void OP_8xy5();
	void OP_8xy6();
	void OP_8xy7();
	void OP_8xyE();
	void OP_9xy0();
	void OP_Annn();
	void OP_Bnnn();
	void OP_Cxkk();
	void OP_Dxyn();
	void OP_Ex9E();
	void OP_ExA1();
	void OP_Fx07();
	void OP_Fx0A();
	void OP_Fx15();
	void OP_Fx18();
	void OP_Fx1E();
	void OP_Fx29();
	void OP_Fx33();
	void OP_Fx55();
	void OP_Fx65();
};

void Chip8::LoadROM(char const *filename)
{
	// Open the file as a stream of binary and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		// Get size of file and allocate a buffer to hold the contents
		std::streampos size = file.tellg();
		char *buffer = new char[size];

		// Go back to the beginning of the file and fill the buffer
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// Load the ROM contents into the Chip8's memory, starting at 0x200
		for (long i = 0; i < size; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		// Free the buffer
		delete[] buffer;
	}
}

void Chip8::Cycle()
{
	// Fetch
	opcode = (memory[pc] << 8u | memory[pc + 1]);
	logOP();

	// Increment the PC before we execute anything
	pc += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		delayTimer--;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		soundTimer--;
	}
}
void Chip8::OP_00E0()
// clear screen
{
	memset(screen, 0, sizeof(screen));
}

void Chip8::OP_00EE()
// RET
{
	--sp;
	pc = stack[sp];
}

void Chip8::OP_1nnn()
// JMP to @nnn
{
	uint16_t address = opcode & 0x0FFFu;
	pc = address;
}

void Chip8::OP_2nnn()
// CALL at @nnn
{
	stack[sp] = pc;
	sp++;

	uint16_t address = opcode & 0XFFFu;
	pc = address;
}

void Chip8::OP_3xkk()
// SE Vx, kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = (opcode & 0x00FFu);
	if (registers[Vx] == value)
	{
		pc += 2;
	}
}

void Chip8::OP_4xkk()
// SNE Vx, kk
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = (opcode & 0x00FFu);
	if (registers[Vx] != value)
	{
		pc += 2;
	}
}

void Chip8::OP_5xy0()
// SE Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] == registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_6xkk()
// LD Vx, kk
{
	uint8_t value = opcode & 0x00FFu;
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = value;
}

void Chip8::OP_7xkk()
// ADD Vx, kk
{
	uint8_t value = opcode & 0x00FFu;
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] += value;
}

void Chip8::OP_8xy0()
// LD Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1()
// OR Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2()
// AND Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3()
// XOR Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[Vx] ^= registers[Vy];
}
void Chip8::OP_8xy4()
// ADD Vx, Vy with carry flag in VF
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	uint8_t result = registers[Vx] + registers[Vy];
	registers[0xF] = result < registers[Vx] ? 1 : 0;

	registers[Vx] = result;
}

void Chip8::OP_8xy5()
// SUB Vx, Vy with VF = NOT borrow
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[0xF] = registers[Vx] > registers[Vy] ? 1 : 0;

	registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6()
// SHR Vx with lost bit in VF
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[0xF] = registers[Vx] & 0x1u;
	registers[Vx] >>= 1;
}

void Chip8::OP_8xy7()
// SUBN Vx, Vy
//  Set Vx = Vy - Vx, set VF = NOT borrow.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	registers[0xF] = registers[Vy] > registers[Vx] ? 1 : 0;

	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE()
// SHL Vx with lost bit in VF
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	registers[0xF] = (registers[Vx] & 0x80) >> 7u;

	registers[Vx] <<= 1;
}

void Chip8::OP_9xy0()
// SNE Vx, Vy
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_Annn()
// LD I, addr
{
	uint16_t address = opcode & 0x0FFFu;
	index = address;
}

void Chip8::OP_Bnnn()
// Bnnn - JP V0, addr
// Jump to location nnn + V0.
{
	uint16_t address = opcode & 0x0FFFu;
	pc = registers[0] + address;
}
void Chip8::OP_Cxkk()
// Cxkk - RND Vx, byte
// Set Vx = random byte AND kk.
{
	uint8_t value = opcode & 0x00FFu;
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = randByte(randGen) & value;
}

void Chip8::OP_Dxyn()
// DRW Vx, Vy, nibble

// Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xPos = registers[Vx] % SCREEN_WIDTH;
	uint8_t yPos = registers[Vy] % SCREEN_HEIGHT;

	registers[0xF] = 0; // if no collision happens VF stays 0

	uint8_t mask = 0x80;

	for (uint8_t row = 0; row < height; row++)
	{
		for (uint8_t col = 0; col < 8; col++)
		{
			uint8_t spritePixel = memory[index + row] & (mask >> col);

			uint16_t x = (xPos + col) % SCREEN_WIDTH;
			uint16_t y = (yPos + row) % SCREEN_HEIGHT;
			uint16_t screenPixelPosition = y * SCREEN_WIDTH + x;

			if (!registers[0xF] && screen[screenPixelPosition] && spritePixel)
			{
				registers[0xF] = 1;
			}
			if (spritePixel)
			{
				screen[screenPixelPosition] ^= 0xFFFFFFFF;
			}
			// else XOR with 0;
		}
	}
}

void Chip8::OP_Ex9E()
// SKP Vx
// Skip next instruction if key with the value of Vx is pressed.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_ExA1()
// SKNP Vx
// Skip next instruction if key with the value of Vx is not pressed.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	uint8_t key = registers[Vx];

	if (!keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_Fx07()
// LD Vx, DT
// Set Vx = delay timer value.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A()
// LD Vx, K
// Wait for a key press, store the value of the key in Vx.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
}

void Chip8::OP_Fx15()
// LD DT, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	delayTimer = registers[Vx];
}

void Chip8::OP_Fx18()
// LD ST, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E()
// ADD I, Vx
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	index += registers[Vx];
}

void Chip8::OP_Fx29()
// LD F, Vx
// Set I = location of sprite for digit Vx.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8;

	index = FONTSET_START_ADDRESS + (5 * registers[Vx]);
}

void Chip8::OP_Fx33()
// LD B, Vx
// Store BCD representation of Vx in memory locations I, I+1, and I+2.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8;

	uint8_t hundreds = (registers[Vx] / 100) % 10;
	uint8_t tens = (registers[Vx] / 10) % 10;
	uint8_t units = registers[Vx] % 10;

	memory[index] = hundreds;
	memory[index + 1] = tens;
	memory[index + 2] = units;
}

void Chip8::OP_Fx55()
// LD [I], Vx
// Store registers V0 through Vx in memory starting at location I.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8;

	for (uint8_t i = 0; i <= Vx; i++)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::OP_Fx65()
// LD Vx, [I]
// Read registers V0 through Vx from memory starting at location I.
{
	uint8_t Vx = (opcode & 0x0F00u) >> 8;

	for (uint8_t i = 0; i <= Vx; i++)
	{
		registers[i] = memory[index + i];
	}
}

void Chip8::logOP()
{

	std::cout << std::hex << pc - 2 << ": " << std::hex << opcode << std::endl;
}
