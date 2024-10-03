#pragma once
#include "Character.cpp"
#include "SurfaceManager.cpp"
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_config.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_video.h>

struct Node {
  Character *data;
  Node *next;
  Node *prev;

public:
  Node(Character *val, Node *nptr = nullptr, Node *pptr = nullptr)
      : data(val), next(nptr), prev(pptr) {}

  ~Node() { delete data; }
};

struct CharacterLinkedList {
  Node *head, *selected;

  SurfaceManager surfaceManager;
  TTF_Font *font;
  SDL_Renderer *renderer;
  int windowWidth = 1000, windowHeight = 0;

  double scale = 1.5;
  double targetScale;
  double manualScale = 0;

public:
  CharacterLinkedList(TTF_Font *font, SDL_Renderer *renderer,
                      SDL_Window *window)
      : font(font), renderer(renderer) {
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    head = selected = new Node(new Character(
        10, 10, '\n', surfaceManager.getCharSurface('\n'), renderer));
  }

  void selectRight() {
    if (selected->next)
      selected = selected->next;
  }

  void selectLeft() {
    if (selected->prev)
      selected = selected->prev;
  }

  void selectUp() {
    Node *ptr = selected;

    while (ptr && ptr->data->ch != '\n')
      ptr = ptr->prev;

    if (ptr == head) {
      selected = head;
      return;
    }

    ptr = ptr->prev;

    while (ptr->data->ch != '\n' &&
           ptr->prev->data->rect->x + ptr->data->rect->w >
               selected->data->rect->x + selected->data->rect->w) {
      ptr = ptr->prev;
    }

    if (ptr->data->ch == '\n')
      selected = ptr;
    else
      selected = ptr->prev;
  }

  void selectDown() {
    Node *ptr = selected;

    if (!ptr->next)
      return;

    if (selected->data->ch == '\n')
      ptr = ptr->next;

    while (ptr && ptr->data->ch != '\n') {
      if (!ptr->next) {
        selected = ptr;
        return;
      }

      ptr = ptr->next;
    }

    if (!ptr->next) {
      selected = ptr;
      return;
    }

    if (ptr->data->rect->x == selected->data->rect->x) {
      if (selected->data->rect->w != 0)
        selected = ptr->next;
      else
        selected = ptr;
      return;
    }

    while (ptr->next && ptr->next->data->ch != '\n' &&
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

    Character *newCharacter = new Character(
        newX, newY, val, surfaceManager.getCharSurface(val), renderer);

    if (selected->next) {
      newNode = new Node(newCharacter, selected->next, selected);
      newNode->prev->next = newNode;
      newNode->next->prev = newNode;
    } else {
      newNode = new Node(newCharacter, nullptr, selected);
      newNode->prev->next = newNode;
    }

    selected = newNode;
  }

  void del() {
    if (!selected->next)
      return;
    selected = selected->next;
    backspace();
  }

  void ctrlDel() {
    ctrlRight();
    ctrlBackspace();
    selectRight();
  }

  void backspace() {
    if (!selected->prev)
      return;

    Node *startPtr = selected->prev;

    selected->prev->next = selected->next;

    if (selected->next)
      selected->next->prev = selected->prev;

    Node *node = selected;
    selected = selected->prev;

    delete node;

    updatePosition(startPtr);
  }

  void ctrlBackspace() {
    Node *ptr = selected;

    if (!ptr->prev)
      return;

    int count = 0;

    while (ptr && (ptr->data->ch == ' ' || ptr->data->ch == '\n')) {
      if (!ptr->prev) {
        selected = ptr;
        updatePosition(selected->prev);
        return;
      }

      ptr->prev->next = ptr->next;

      if (ptr->next)
        ptr->next->prev = ptr->prev;

      ptr = ptr->prev;
      count++;
    }

    if (count <= 1) {
      while (ptr->prev && ptr->data->ch != ' ' && ptr->prev->data->ch != '\n') {
        ptr->prev->next = ptr->next;

        if (ptr->next)
          ptr->next->prev = ptr->prev;

        ptr = ptr->prev;
      }
    }

    if (ptr->prev && (ptr->data->ch == ' ' || ptr->prev->data->ch == '\n')) {
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
    while (ptr->next &&
           (ptr->next->data->ch == ' ' || ptr->next->data->ch == '\n')) {
      count++;
      ptr = ptr->next;
    }

    if (count <= 1)
      while (ptr->next &&
             (ptr->next->data->ch != ' ' && ptr->next->data->ch != '\n')) {
        ptr = ptr->next;
      }

    selected = ptr;
  }

  void ctrlLeft() {
    Node *ptr = selected;

    if (!ptr->prev)
      return;

    int count = 0;
    while (ptr && (ptr->data->ch == ' ' || ptr->data->ch == '\n')) {
      count++;
      ptr = ptr->prev;
    }

    if (count <= 1) {
      while (ptr->prev && (ptr->data->ch != ' ' && ptr->data->ch != '\n')) {
        ptr = ptr->prev;
      }
    }

    selected = ptr;
  }

  void updatePosition(Node *startPtr) {
    if (!startPtr || !startPtr->next) {
      return;
    }

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

      if (newX > maxWidth && ptr->data->ch != '\n')
        maxWidth = rect->x;

      if (ptr->prev && (ptr->data->ch == '\n')) {
        newX = head->data->rect->x;
        newY += rect->h;
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
