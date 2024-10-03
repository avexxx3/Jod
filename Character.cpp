#pragma once
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>

struct Character {
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect *rect;
  char ch;

  Character(int x, int y, char c, SDL_Surface *surface, SDL_Renderer *renderer)
      : surface(surface) {
    rect = new SDL_Rect;

    ch = c;

    rect->x = x;
    rect->y = y;
    rect->h = surface->h;
    rect->w = (c == '\n') ? 0 : surface->w;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
  }

  void updatePosition(int x, int y, SDL_Renderer *renderer) {
    rect->x = x;
    rect->y = y;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
  }

  ~Character() { delete rect; }
};