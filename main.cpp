#include <iostream>
#include <chrono>
#include <iomanip>
#include <stdio.h>
#include "platform.cpp"
#include "chip8.cpp"

const int VIDEO_WIDTH = 64;
const int VIDEO_HEIGHT = 32;

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const *romFilename = argv[3];

    Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip8;

    chip8.LoadROM(romFilename);

    // for deletion later
    // for (long i = 0x200; i <= 0x400; i += 2)
    // {
    //     // std::cout << std::hex << opcode << std::endl;
    //     printf("%x\n", chip8.memory[i] << 8u | chip8.memory[i + 1]);
    //     // std::cout << std::hex << chip8.memory[i] << std::endl;
    // }

    int videoPitch = sizeof(chip8.screen[0]) * VIDEO_WIDTH;
    // int videoPitch = 4 * 64;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    chip8.OP_00E0();
    while (!quit)
    {
        quit = platform.ProcessInput(chip8.keypad);

        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

        if (dt > cycleDelay)
        {
            lastCycleTime = currentTime;

            chip8.Cycle();

            platform.Update(chip8.screen, videoPitch);
        }
    }

    return 0;
}
