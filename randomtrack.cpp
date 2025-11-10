#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

int main() {
    // Random number 1–8
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int n = std::rand() % 8 + 1;
    std::cout << "Random number = " << n << std::endl;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL init error: " << SDL_GetError() << "\n";
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer error: " << Mix_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    //to string
    std::string filename = "Sound_Track/Track0" + std::to_string(n) + ".mp3";

    // Load the track
    Mix_Music* music = Mix_LoadMUS(filename.c_str());
    if (!music) {
        std::cerr << "Could not load " << filename << ": " << Mix_GetError() << "\n";
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Play the track (0 = play once)
    if (Mix_PlayMusic(music, 0) == -1) {
        std::cerr << "Play error: " << Mix_GetError() << "\n";
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Wait
    while (Mix_PlayingMusic()) {
        SDL_Delay(100);
    }

    // memory clean
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}
