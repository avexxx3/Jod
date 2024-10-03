#pragma once
#include "Color.cpp"
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>
class SurfaceManager {
  std::unordered_map<char, SDL_Surface *> surfaceMap;
  TTF_Font *font;

  SDL_Surface *makeNewSurface(char ch) {
    char cStr[2] = {ch, '\0'};
    SDL_Surface *surface = TTF_RenderText_Blended(font, cStr, Color::White);
    surfaceMap.insert({ch, surface});
    return surface;
  }

public:
  SurfaceManager() {
    TTF_Init();
    font = TTF_OpenFont("res/VictorMono-Regular.ttf", 48);
  }

  SDL_Surface *getCharSurface(char ch) {
    if (!surfaceMap.count(ch))
      return makeNewSurface(ch);

    return surfaceMap.at(ch);
  }
};
