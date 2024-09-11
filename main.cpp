#pragma once
#include "sdl_utils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <algorithm>
#include <string>
#include <vector>

namespace Color {
SDL_Color White = {255, 255, 255};
SDL_Color Black = {0, 0, 0};
} // namespace Color

SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;

struct TextLine {
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect rect;
  std::string text;

  TextLine(int y) {
    surface = TTF_RenderText_Blended(font, " ", Color::White);
    rect.x = 10;
    rect.y = y;
    rect.h = surface->h;
    rect.w = 0;
  }

  void updateTexture() {
    surface = TTF_RenderText_Blended(font, text.c_str(), Color::White);
    texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (text.empty())
      rect.w = 0;
    else
      rect.w = surface->w;
  }

  void appendChar(char _char) {
    text += _char;
    updateTexture();
  }

  void appendTab() {
    text += "  ";
    updateTexture();
  }

  bool trimChar() {
    if (text.empty())
      return false;

    text.erase(text.size() - 1);
    updateTexture();

    return true;
  }

  bool bigTrim() {
    if (text.empty())
      return false;

    int count = 0;
    int size = text.size();

    while (text[size - 1] == ' ') {
      count++;
      text.erase(size - 1);
      size--;
    }

    if (count <= 1) {
      while (size > 0 && text[size - 1] != ' ') {
        text.erase(size - 1);
        size--;
      }
    }

    updateTexture();

    return true;
  }
};

int main() {
  if (!sdl_utils_Init("SDL Tutorial", &window, &renderer, &font, 0))
    return 0;

  std::vector<TextLine *> lines = {};

  TextLine *currentLine = new TextLine(10);
  lines.emplace_back(currentLine);

  SDL_Rect blinker;
  bool ctrl = false;

  while (1) {
    blinker.x = currentLine->rect.w + 10;
    blinker.y = currentLine->rect.y;
    blinker.h = currentLine->rect.h;
    blinker.w = 4;

    SDL_RenderClear(renderer);
    for (auto &line : lines)
      SDL_RenderCopy(renderer, line->texture, NULL, &line->rect);

    SDL_SetRenderDrawColor(renderer, Color::White.r, Color::White.g,
                           Color::White.b, 255);

    SDL_RenderFillRect(renderer, &blinker);

    SDL_SetRenderDrawColor(renderer, Color::Black.r, Color::Black.g,
                           Color::Black.b, 0);

    SDL_RenderPresent(renderer);

    SDL_Event event;

    if (SDL_PollEvent(&event)) {

      switch (event.type) {
      case SDL_QUIT:
        return 1;

      case SDL_TEXTINPUT:
        if (currentLine->text.size() == 104) {
          currentLine = new TextLine(currentLine->rect.y + currentLine->rect.h);
          lines.emplace_back(currentLine);
        }

        currentLine->appendChar(*event.text.text);
        break;

      case SDL_KEYUP:
        switch (event.key.keysym.sym) {
        case SDLK_LCTRL:
          ctrl = false;
          break;
        }
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_LCTRL:
          ctrl = true;
          break;

        case SDLK_BACKSPACE:
          if (ctrl) {
            if (currentLine->bigTrim() || lines.size() == 1)
              continue;
          } else if (currentLine->trimChar() || lines.size() == 1)
            continue;

          lines.pop_back();
          currentLine = lines.back();
          break;

        case SDLK_TAB:
          currentLine->appendTab();
          break;

        case SDLK_RSHIFT:
          currentLine = new TextLine(currentLine->rect.y + currentLine->rect.h);
          lines.emplace_back(currentLine);
          break;
        }
      }
    }
  }

  sdl_utils_Quit(window, renderer);
  return 0;
}