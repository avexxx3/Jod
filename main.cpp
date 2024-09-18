#pragma once
#include "DoublyLinkedList.cpp"
#include "SDLUtils.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_opengl.h>
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

  int trio = 0;

  SDL_Rect newT;
  newT.h = 10000;
  newT.w = 10000;
  newT.x = newT.y = 0;

  double scale = 1;

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

    SDL_RenderSetScale(renderer, linkedList.scale, linkedList.scale);
    SDL_RenderSetViewport(renderer, &newT);

    if (trio++ == 100) {
      linkedList.sway();
      trio = 0;
    }

    SDL_RenderPresent(renderer);

    SDL_Event event;

    if (SDL_PollEvent(&event)) {

      switch (event.type) {
      case SDL_QUIT:
        return 1;

      case SDL_TEXTINPUT:
        if (*event.text.text == '-' && ctrl) {
          linkedList.scaleManually(false);
          break;
        }

        if (*event.text.text == '=' && ctrl) {
          linkedList.scaleManually(true);
          break;
        }

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
          ctrl ? linkedList.ctrlLeft() : linkedList.selectPrev();
          break;

        case SDLK_RIGHT:
          ctrl ? linkedList.ctrlRight() : linkedList.selectNext();
          break;

        case SDLK_BACKSPACE:
          ctrl ? linkedList.ctrlBackspace() : linkedList.backspace();
          break;

        case SDLK_TAB:
          linkedList.tab();
          break;

        case SDLK_UP:
          linkedList.selectUp();
          break;

        case SDLK_DOWN:
          linkedList.selectDown();
          break;

        case SDLK_RSHIFT:
          linkedList.insert('\n');
          break;

        case SDLK_PLUS:
          linkedList.scaleManually(true);
          break;

        case SDLK_MINUS:
          linkedList.scaleManually(false);
          break;
        }
      }
    }
  }

  sdl_utils_Quit(window, renderer);
  return 0;
}