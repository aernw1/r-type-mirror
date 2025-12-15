/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** POC SDL2
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "=== SDL2 Proof of Concept ===" << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image initialization failed: " << IMG_GetError() << std::endl;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
    }

    SDL_Window* window = SDL_CreateWindow(
        "SDL2 PoC - R-Type",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    std::cout << "Window and renderer created successfully" << std::endl;

    SDL_Texture* texture = IMG_LoadTexture(renderer, "assets/ship.png");
    if (!texture) {
        std::cout << "Warning: Could not load ship.png, using colored rectangle" << std::endl;
    }

    SDL_Rect spriteRect = {400, 300, 50, 50};

    Mix_Chunk* sound = Mix_LoadWAV("assets/shoot.wav");
    if (sound) {
        std::cout << "Sound loaded successfully" << std::endl;
    } else {
        std::cout << "Warning: Could not load shoot.wav" << std::endl;
    }

    std::cout << "Controls: Arrow keys to move, Space to play sound, ESC to quit" << std::endl;

    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float speed = 200.f;

    while (running)
    {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    running = false;
                if (event.key.keysym.sym == SDLK_SPACE && sound)
                    Mix_PlayChannel(-1, sound, 0);
            }
        }

        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        float moveX = 0.f, moveY = 0.f;

        if (keystate[SDL_SCANCODE_LEFT])
            moveX -= speed * deltaTime;
        if (keystate[SDL_SCANCODE_RIGHT])
            moveX += speed * deltaTime;
        if (keystate[SDL_SCANCODE_UP])
            moveY -= speed * deltaTime;
        if (keystate[SDL_SCANCODE_DOWN])
            moveY += speed * deltaTime;

        spriteRect.x += static_cast<int>(moveX);
        spriteRect.y += static_cast<int>(moveY);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (texture) {
            SDL_RenderCopy(renderer, texture, NULL, &spriteRect);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_RenderFillRect(renderer, &spriteRect);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    if (texture) SDL_DestroyTexture(texture);
    if (sound) Mix_FreeChunk(sound);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();

    std::cout << "SDL2 PoC completed successfully" << std::endl;
    return 0;
}
