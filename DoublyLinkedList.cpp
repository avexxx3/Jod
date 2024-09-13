#pragma once
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>

namespace Color {
SDL_Color White = {255, 255, 255};
SDL_Color Black = {0, 0, 0};
} // namespace Color

struct Character {
  SDL_Surface *surface;
  SDL_Texture *texture;
  SDL_Rect *rect;
  char cStr[2];

  Character(int x, int y, char c, TTF_Font *font, SDL_Renderer *renderer) {
    rect = new SDL_Rect;

    cStr[0] = c;
    cStr[1] = '\0';

    surface = TTF_RenderText_Blended(font, cStr, Color::White);
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

  ~Character() {
    delete surface;
    delete rect;
  }
};

struct CharacterLinkedList {
  struct Node {
    Character *data;
    Node *next;
    Node *prev;

  public:
    Node(Character *val, Node *nptr = nullptr, Node *pptr = nullptr)
        : data(val), next(nptr), prev(pptr) {}

    ~Node() { delete data; }
  };

  Node *head, *tail, *selected;

  TTF_Font *font;
  SDL_Renderer *renderer;
  int windowWidth = 1000, windowHeight = 0;

public:
  CharacterLinkedList(TTF_Font *font, SDL_Renderer *renderer,
                      SDL_Window *window)
      : font(font), renderer(renderer) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    head = tail = selected =
        new Node(new Character(10, 10, '\n', font, renderer));
  }

  void selectNext() {
    if (selected->next)
      selected = selected->next;
  }

  void selectPrev() {
    if (selected->prev)
      selected = selected->prev;
  }

  void tab() {
    for (int i = 0; i < 3; i++)
      insert(' ');
  }

  void insert(char val) {
    Node *newNode;

    SDL_Rect *rect = selected->data->rect;

    int newX = (selected == head) ? 10 : rect->x + rect->w;
    int newY = rect->y;

    if (newX >= windowWidth - rect->w || val == '\n') {
      newX = 10;
      newY += rect->h;
    }

    Character *newCharacter = new Character(newX, newY, val, font, renderer);

    if (selected->next) {
      newNode = new Node(newCharacter, selected->next, selected);
      newNode->prev->next = newNode;
      newNode->next->prev = newNode;
    } else {
      newNode = new Node(newCharacter, nullptr, selected);
      newNode->prev->next = newNode;
    }

    selected = newNode;
    updatePosition(selected);
  }

  void backspace() {
    if (!selected->prev)
      return;

    selected->prev->next = selected->next;

    if (selected->next)
      selected->next->prev = selected->prev;

    Node *node = selected;
    selected = selected->prev;

    delete node;

    updatePosition(selected);
  }

  void ctrlBackspace() {
    Node *ptr = selected;
    Node *ptrEnd = selected;

    if (!ptr->prev)
      return;

    int count = 0;

    while (ptr->prev && (ptr->prev->data->cStr[0] == ' ' ||
                         ptr->prev->data->cStr[0] == '\n')) {
      ptr->prev->next = ptr->next;

      if (ptr->next)
        ptr->next->prev = ptr->prev;

      ptr = ptr->prev;
      count++;
    }

    if (count <= 1) {
      while (ptr->prev && ptr->data->cStr[0] != ' ' &&
             ptr->prev->data->cStr[0] != '\n') {
        ptr->prev->next = ptr->next;

        if (ptr->next)
          ptr->next->prev = ptr->prev;

        ptr = ptr->prev;
      }
    }

    if (ptr->prev &&
        (ptr->data->cStr[0] == ' ' || ptr->prev->data->cStr[0] == '\n')) {
      ptr->prev->next = ptr->next;

      if (ptr->next)
        ptr->next->prev = ptr->prev;

      ptr = ptr->prev;
    }

    selected = ptr;

    updatePosition(selected);
  }

  void updatePosition(Node *startPtr) {
    if (!startPtr || !startPtr->next)
      return;

    if (!startPtr->prev) {
    }

    for (Node *ptr = startPtr; ptr; ptr = ptr->next) {

      SDL_Rect *rect;

      if (ptr->prev) {
        rect = ptr->prev->data->rect;
      } else {
        rect = new SDL_Rect;
        rect->h = startPtr->next->data->rect->h;
        rect->w = startPtr->next->data->rect->w;
        rect->x = 10 - rect->w;
        rect->y = 10;
      }

      int newX = rect->x + rect->w;
      int newY = rect->y;

      if (ptr->prev &&
          (newX >= windowWidth - rect->w || ptr->data->cStr[0] == '\n')) {

        while (ptr->prev && ptr->prev->data->cStr[0] != ' ') {
          ptr = ptr->prev;
        }

        newX = 10;
        newY += rect->y;

        if (ptr->prev) {
          ptr->data->updatePosition(newX, newY, renderer);
          updatePosition(ptr);
          return;
        }
      }

      ptr->data->updatePosition(newX, newY, renderer);
    }
  }

  ~CharacterLinkedList() {
    Node *curr = head;
    Node *prev;
    while (curr != nullptr) {
      prev = curr;
      curr = curr->next;
      delete prev;
    }
  }
};