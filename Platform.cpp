#include <iostream>

#define SDL_MAIN_NOIMPL
#include <SDL3/SDL.h>

class Platform
{
public:
    Platform(char const *title, int windowWidth, int windowHeight, int textureWidth, int textureHeight)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            std::cout << "error in initializing SDL: " << SDL_GetError() << std::endl;
            std::exit(1);
        };

        window = SDL_CreateWindow(title, windowWidth, windowHeight, SDL_WINDOW_RESIZABLE);

        renderer = SDL_CreateRenderer(window, nullptr);

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);

        if (window == NULL || renderer == NULL || texture == NULL)
        {
            std::cout << "error in constructing platform: " << SDL_GetError() << std::endl;
            std::exit(1);
        }

        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST); // disables blending between pixels
    }

    ~Platform()
    {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Update(void const *buffer, int pitch)
    {
        SDL_UpdateTexture(texture, nullptr, buffer, pitch);

        // buffer: the raw pixel data in the format of the texture.
        // pitch: the number of bytes in a row of pixel data, including padding between lines.

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    bool ProcessInput(uint8_t *keys)
    {
        bool quit = false;

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
            {
                quit = true;
            }
            break;

            case SDL_EVENT_KEY_DOWN:
            {
                switch (event.key.key)
                {
                case SDLK_ESCAPE:
                {
                    quit = true;
                }
                break;

                case SDLK_X:
                {
                    keys[0] = 1;
                }
                break;

                case SDLK_1:
                {
                    keys[1] = 1;
                }
                break;

                case SDLK_2:
                {
                    keys[2] = 1;
                }
                break;

                case SDLK_3:
                {
                    keys[3] = 1;
                }
                break;

                case SDLK_A:
                {
                    keys[4] = 1;
                }
                break;

                case SDLK_Z:
                {
                    keys[5] = 1;
                }
                break;

                case SDLK_E:
                {
                    keys[6] = 1;
                }
                break;

                case SDLK_Q:
                {
                    keys[7] = 1;
                }
                break;

                case SDLK_S:
                {
                    keys[8] = 1;
                }
                break;

                case SDLK_D:
                {
                    keys[9] = 1;
                }
                break;

                case SDLK_W:
                {
                    keys[0xA] = 1;
                }
                break;

                case SDLK_C:
                {
                    keys[0xB] = 1;
                }
                break;

                case SDLK_4:
                {
                    keys[0xC] = 1;
                }
                break;

                case SDLK_R:
                {
                    keys[0xD] = 1;
                }
                break;

                case SDLK_F:
                {
                    keys[0xE] = 1;
                }
                break;

                case SDLK_V:
                {
                    keys[0xF] = 1;
                }
                break;
                }
            }
            break;

            case SDL_EVENT_KEY_UP:
            {
                switch (event.key.key)
                {
                case SDLK_X:
                {
                    keys[0] = 0;
                }
                break;

                case SDLK_1:
                {
                    keys[1] = 0;
                }
                break;

                case SDLK_2:
                {
                    keys[2] = 0;
                }
                break;

                case SDLK_3:
                {
                    keys[3] = 0;
                }
                break;

                case SDLK_A:
                {
                    keys[4] = 0;
                }
                break;

                case SDLK_Z:
                {
                    keys[5] = 0;
                }
                break;

                case SDLK_E:
                {
                    keys[6] = 0;
                }
                break;

                case SDLK_Q:
                {
                    keys[7] = 0;
                }
                break;

                case SDLK_S:
                {
                    keys[8] = 0;
                }
                break;

                case SDLK_D:
                {
                    keys[9] = 0;
                }
                break;

                case SDLK_W:
                {
                    keys[0xA] = 0;
                }
                break;

                case SDLK_C:
                {
                    keys[0xB] = 0;
                }
                break;

                case SDLK_4:
                {
                    keys[0xC] = 0;
                }
                break;

                case SDLK_R:
                {
                    keys[0xD] = 0;
                }
                break;

                case SDLK_F:
                {
                    keys[0xE] = 0;
                }
                break;

                case SDLK_V:
                {
                    keys[0xF] = 0;
                }
                break;
                }
            }
            break;
            }
        }

        return quit;
    }

private:
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};
};
