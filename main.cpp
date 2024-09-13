#pragma once
#include "DoublyLinkedList.cpp"
#include "SDLUtils.h"
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
SDL_Window *window;
SDL_Renderer *renderer;
TTF_Font *font;

int main() {
  if (!sdl_utils_Init("SDL Tutorial", &window, &renderer, &font, 0))
    return 0;

  CharacterLinkedList linkedList(font, renderer, window);
  bool ctrl = false;

  SDL_Rect blinker;

  while (1) {
    SDL_RenderClear(renderer);

    for (CharacterLinkedList::Node *ptr = linkedList.head; ptr != nullptr;
         ptr = ptr->next) {
      SDL_RenderCopy(renderer, ptr->data->texture, NULL, ptr->data->rect);
      if (ptr == linkedList.selected) {
        SDL_SetRenderDrawColor(renderer, Color::White.r, Color::White.g,
                               Color::White.b, 255);

        blinker.x = ptr->data->rect->x + ptr->data->rect->w - 1;
        blinker.y = ptr->data->rect->y;
        blinker.h = ptr->data->rect->h;
        blinker.w = 4;

        SDL_RenderFillRect(renderer, &blinker);

        SDL_SetRenderDrawColor(renderer, Color::Black.r, Color::Black.g,
                               Color::Black.b, 0);
      }
    }

    SDL_RenderPresent(renderer);

    SDL_Event event;

    if (SDL_PollEvent(&event)) {

      switch (event.type) {
      case SDL_QUIT:
        return 1;

      case SDL_TEXTINPUT:
        linkedList.insert(*event.text.text);
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

        case SDLK_LEFT:
          linkedList.selectPrev();
          break;

        case SDLK_RIGHT:
          linkedList.selectNext();
          break;

        case SDLK_BACKSPACE:
          ctrl ? linkedList.ctrlBackspace() : linkedList.backspace();
          break;

        case SDLK_TAB:
          linkedList.tab();
          break;

        case SDLK_RSHIFT:
          linkedList.insert('\n');
          break;
        }
      }
    }
  }

  sdl_utils_Quit(window, renderer);
  return 0;
}