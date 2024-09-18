#pragma once
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>
#include <cstdlib>
#include <iostream>

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

struct LinkedList {
  struct Node {
    Character *data;
    Node *next;
    Node *prev;

  public:
    Node(Character *val, Node *nptr = nullptr, Node *pptr = nullptr)
        : data(val), next(nptr), prev(pptr) {}

    ~Node() { delete data; }
  };

  Node *head, *selected;
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

  Node *head, *selected;

  TTF_Font *font;
  SDL_Renderer *renderer;
  int windowWidth = 1000, windowHeight = 0;

  int targetX = 10;
  int targetY = 10;
  bool posX = 0;
  bool posY = 0;

  double scale = 1.5;
  double manualScale = 0;

public:
  CharacterLinkedList(TTF_Font *font, SDL_Renderer *renderer,
                      SDL_Window *window)
      : font(font), renderer(renderer) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    head = selected = new Node(new Character(10, 10, '\n', font, renderer));
  }

  void sway() {
    return;

    SDL_Rect *rect = head->data->rect;
    if (rect->y == targetY && rect->y == targetY) {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      srand((time_t)ts.tv_nsec);
      targetX = (rand() % 6 * ((rand() % 2) ? -1 : 1));
      targetY = (rand() % 6 * ((rand() % 2) ? -1 : 1));

      posX = targetX > rect->x;
      posY = targetY > rect->y;
      return;
    }

    if (rect->x != targetX)
      if (posX) {
        rect->x++;
      } else {
        rect->x--;
      }

    if (rect->y != targetY)
      if (posY) {
        rect->y++;
      } else {
        rect->y--;
      }
    updatePosition(head);
  }

  void selectNext() {
    if (selected->next)
      selected = selected->next;
  }

  void selectPrev() {
    if (selected->prev)
      selected = selected->prev;
  }

  void selectUp() {
    Node *ptr = selected;

    while (ptr && ptr->data->cStr[0] != '\n')
      ptr = ptr->prev;

    if (ptr == head) {
      selected = head;
      return;
    }

    // if (ptr->data->rect->x == selected->data->rect->x) {
    //   if (selected->data->rect->w != 0)
    //     selected = ptr->next;
    //   else
    //     selected = ptr;
    //   return;
    // }

    ptr = ptr->prev;

    while (ptr->data->cStr[0] != '\n' &&
           ptr->prev->data->rect->x + ptr->data->rect->w >
               selected->data->rect->x + selected->data->rect->w) {
      ptr = ptr->prev;
    }

    if (ptr->data->cStr[0] == '\n')
      selected = ptr;
    else
      selected = ptr->prev;
  }

  void selectDown() {
    Node *ptr = selected;

    if (!ptr->next)
      return;

    if (selected->data->cStr[0] == '\n')
      ptr = ptr->next;

    while (ptr && ptr->data->cStr[0] != '\n') {
      if (!ptr->next) {
        selected = ptr;
        return;
      }

      ptr = ptr->next;
    }

    if (ptr->data->rect->x == selected->data->rect->x) {
      if (selected->data->rect->w != 0)
        selected = ptr->next;
      else
        selected = ptr;
      return;
    }

    ptr = ptr->next;

    while (ptr && ptr->data->cStr[0] != '\n' &&
           ptr->data->rect->x < selected->data->rect->x) {
      ptr = ptr->next;
    }

    selected = ptr;
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

    if (val == '\n') {
      newX = head->data->rect->x;
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
    updatePosition(head);
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

    updatePosition(head);
  }

  void ctrlBackspace() {
    Node *ptr = selected;

    if (!ptr->prev)
      return;

    int count = 0;

    while (ptr && (ptr->data->cStr[0] == ' ' || ptr->data->cStr[0] == '\n')) {
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

    updatePosition(head);
  }

  void ctrlRight() {
    Node *ptr = selected;

    if (!ptr->next)
      return;

    int count = 0;
    while (ptr->next && (ptr->next->data->cStr[0] == ' ' ||
                         ptr->next->data->cStr[0] == '\n')) {
      count++;
      ptr = ptr->next;
    }

    if (count <= 1)
      while (ptr->next && (ptr->next->data->cStr[0] != ' ' &&
                           ptr->next->data->cStr[0] != '\n')) {
        ptr = ptr->next;
      }

    selected = ptr;
  }

  void ctrlLeft() {
    Node *ptr = selected;

    if (!ptr->prev)
      return;

    int count = 0;
    while (ptr && (ptr->data->cStr[0] == ' ' || ptr->data->cStr[0] == '\n')) {
      count++;
      ptr = ptr->prev;
    }

    if (count <= 1) {
      while (ptr->prev &&
             (ptr->data->cStr[0] != ' ' && ptr->data->cStr[0] != '\n')) {
        ptr = ptr->prev;
      }
    }

    selected = ptr;
  }

  void updatePosition(Node *startPtr) {
    if (!startPtr || !startPtr->next)
      return;

    int maxWidth = -1;

    for (Node *ptr = startPtr; ptr; ptr = ptr->next) {
      SDL_Rect *rect;

      if (ptr->prev) {
        rect = ptr->prev->data->rect;
      } else {
        continue;
      }

      int newX = rect->x + rect->w;
      int newY = rect->y;

      if (newX > maxWidth && ptr->data->cStr[0] != '\n')
        maxWidth = rect->x;

      if (ptr->prev && (ptr->data->cStr[0] == '\n')) {
        newX = head->data->rect->x;
        newY += rect->h;
      }

      ptr->data->updatePosition(newX, newY, renderer);
    }

    scale = (1260.0 / maxWidth) + manualScale;

    if (scale > 1.5)
      scale = 1.5;
  }

  void scaleManually(const bool increase) {
    manualScale = increase ? manualScale + 0.01 : manualScale - 0.01;
    updatePosition(head);
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