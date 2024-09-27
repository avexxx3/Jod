#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fstream>
#include <stdio.h>

inline bool sdl_utils_Init(const char *title, SDL_Window **window,
                           SDL_Renderer **renderer, TTF_Font **font) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return 0;
  }

  SDL_Init(SDL_INIT_TIMER);
  SDL_Init(SDL_INIT_AUDIO);

  if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
    printf("Warning: Linear texture filtering not enabled!");
  }

  SDL_DisplayMode DisplayMode;

  if (SDL_GetDesktopDisplayMode(0, &DisplayMode) != 0) {
    printf("SDL_GetDesktopDisplayMode failed: %s\n", SDL_GetError());
    return 0;
  }

  *window =
      SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                       DisplayMode.w, DisplayMode.h, SDL_WINDOW_SHOWN);
  if (window == 0) {
    printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
    return 0;
  }

  *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == 0) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return 0;
  }
  SDL_SetRenderDrawColor(*renderer, 0, 0, 0, 0xFF);

  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    printf("SDL_image could not initialize! SDL_image Error: %s\n",
           IMG_GetError());
    return 0;
  }

  SDL_SetWindowFullscreen(*window, SDL_WINDOW_FULLSCREEN);

  TTF_Init();

  *font = TTF_OpenFont("VictorMono-Regular.ttf", 48);

  if (font == 0) {
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return 0;
  }

  return 1;
}

inline void sdl_utils_Quit(SDL_Window *window, SDL_Renderer *renderer) {
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (window)
    SDL_DestroyWindow(window);

  IMG_Quit();
  SDL_Quit();
}
