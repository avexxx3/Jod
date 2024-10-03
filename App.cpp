#include "CharacterLinkedList.cpp"
#include "SDLUtils.cpp"
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
#include <ctime>

struct App {
  CharacterLinkedList *linkedList; // The main character linked list
  SDL_Window *window;              // The window used to show all changes
  SDL_Renderer
      *renderer;  // The renderer responsible for rendering all screen elements
  TTF_Font *font; // The font responsible for the text (VictorMono-Regular.ttf)

  SDL_Rect cursorRect;   // The rectangle that represents the cursor.
  SDL_Rect viewportRect; // The view encasing the rendered objects
  SDL_Rect swayRect;

  bool ctrlHeld = false; // Boolean showing held state of the CTRL key
  bool capsLock;         // Boolean showing CAPS LOCK state of the system
  bool maxScaleReached = false;

  const char *windowTitle = "Jod";

  int windowWidth;
  int windowHeight;

  int targetSwayX = 10;
  int targetSwayY = 10;
  int swayX = 0;
  int swayY = 0;

  bool posX;
  bool posY;

  double scale = 0.75;
  double targetScale = 0.75;

  int targetViewportX;
  int targetViewportY;
  int targetCursorX;
  int targetCursorY;

  float cursorXVel = 5;
  float cursorYVel = 5;
  float viewportXVel = 5;
  float viewportYVel = 5;

  float scaleSpeed = 0.001;
  bool zoomed = false;

  SDL_AudioDeviceID deviceId;
  SDL_AudioSpec wavSpec;
  Uint32 wavLength;
  Uint8 *wavBuffer;

  ~App() { SDL_FreeWAV(wavBuffer); }

  App() {
    SDLUtilsInit(windowTitle, &window, &renderer);
    linkedList = new CharacterLinkedList(font, renderer, window);
    capsLock = SDL_GetModState() & KMOD_CAPS;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    SDL_LoadWAV("res/click.wav", &wavSpec, &wavBuffer, &wavLength);
    deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
    viewportRect.h = viewportRect.w = 10000;
    viewportRect.x = viewportRect.y = 10;

    cursorRect.x = linkedList->selected->data->rect->x +
                   linkedList->selected->data->rect->w - 1;
    cursorRect.y = linkedList->selected->data->rect->y;
    cursorRect.h = linkedList->selected->data->rect->h;
    cursorRect.w = 4;

    viewportRect.x = swayRect.x = 0;

    viewportRect.y = swayRect.y = targetViewportY =
        (1 / targetScale) * (windowHeight / 2.0) -
        (linkedList->selected->data->rect->y) -
        linkedList->selected->data->rect->h / 2.0;
    setScale();
  }

  void shakeScreen() {
    if (swayX == targetSwayX && swayY == targetSwayY) {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      srand((time_t)ts.tv_nsec);
      targetSwayX = (rand() % 5 * ((rand() % 2) ? -1 : 1));
      targetSwayY = (rand() % 5 * ((rand() % 2) ? -1 : 1));

      posX = targetSwayX > swayX;
      posY = targetSwayY > swayY;
      return;
    }

    if (swayX != targetSwayX)
      posX ? swayX++ : swayX--;

    if (swayY != targetSwayY)
      posY ? swayY++ : swayY--;
  }

  void setScale() {
    if (maxScaleReached || zoomed)
      return;

    Node *ptr = linkedList->head;

    double newScale = 0;
    double minScale = 0.75;

    while (ptr->next) {
      if (ptr->next->data->ch == '\n' || !ptr->next->next) {
        newScale = ((windowWidth * 0.922) / 2) /
                   (ptr->data->rect->x + (10 - targetSwayX));

        if (newScale < minScale)
          minScale = newScale;
      }

      ptr = ptr->next;
    }

    if (minScale > 0.5)
      targetScale = minScale;

    if (minScale <= 0.5) {
      maxScaleReached = true;
      if (targetScale > 0.5)
        targetScale = 0.5;
    }

    scaleSpeed = (targetScale - scale) / 15.0;
  }

  void zoom(bool zoomIn) {
    zoomed = true;

    if ((!zoomIn && targetScale > 1.5) || (zoomIn && targetScale < 0.33))
      return;

    zoomIn ? targetScale *= 0.95 : targetScale *= 1.05;
    scaleSpeed = (targetScale - scale) / 15.0;
  }

  float radius = 4.0f;
  float angle = 0.0f;
  float speed = 0.04f;

  void sway() {
    swayX = radius * cos(angle);
    swayY = radius * sin(angle);

    angle += speed;
    if (angle >= 2 * M_PI) {
      angle -= 2 * M_PI;
    }
  }

  void setSway() {
    swayRect.h = viewportRect.h;
    swayRect.w = viewportRect.w;
    swayRect.x = viewportRect.x + swayX;
    swayRect.y = viewportRect.y + swayY;
  }

  void applyScale() {
    if (targetScale == scale)
      return;

    scale += scaleSpeed;

    if ((scaleSpeed < 0 && scale < targetScale) ||
        (scaleSpeed > 0 && scale > targetScale))
      scale = targetScale;
  }

  void clickAudio() {
    SDL_ClearQueuedAudio(deviceId);
    SDL_QueueAudio(deviceId, wavBuffer, wavLength);
    SDL_PauseAudioDevice(deviceId, 0);
  }

  void centerCursor() {
    targetCursorX = linkedList->selected->data->rect->x +
                    linkedList->selected->data->rect->w - 1;

    targetCursorY = linkedList->selected->data->rect->y;
    cursorRect.h = linkedList->selected->data->rect->h;
    cursorRect.w = 4;

    targetViewportX = (1 / targetScale) * (windowWidth / 2.0) -
                      (linkedList->selected->data->rect->x) -
                      linkedList->selected->data->rect->w / 2.0;

    targetViewportY = (1 / targetScale) * (windowHeight / 2.0) -
                      (linkedList->selected->data->rect->y) -
                      linkedList->selected->data->rect->h / 2.0;

    cursorXVel = (targetCursorX - cursorRect.x) / 15.0;
    cursorYVel = (targetCursorY - cursorRect.y) / 15.0;
    if (cursorXVel > 0 && cursorXVel < 1)
      cursorXVel = 1;
    else if (cursorXVel < 0 && cursorXVel > -1)
      cursorXVel = -1;

    if (cursorYVel > 0 && cursorYVel < 1)
      cursorYVel = 1;
    else if (cursorYVel < 0 && cursorYVel > -1)
      cursorYVel = -1;

    viewportXVel = (targetViewportX - viewportRect.x) / 15.0;
    viewportYVel = (targetViewportY - viewportRect.y) / 15.0;

    if (viewportXVel > 0 && viewportXVel < 1)
      viewportXVel = 1;
    else if (viewportXVel < 0 && viewportXVel > -1)
      viewportXVel = -1;

    if (viewportYVel > 0 && viewportYVel < 1)
      viewportYVel = 1;
    else if (viewportYVel < 0 && viewportYVel > -1)
      viewportYVel = -1;
  }

  void smoothTransitionCursor() {
    if (viewportRect.x != targetViewportX) {
      viewportRect.x += viewportXVel;

      if ((viewportXVel < 0 && viewportRect.x < targetViewportX) ||
          (viewportXVel > 0 && viewportRect.x > targetViewportX))
        viewportRect.x = targetViewportX;
    }

    if (viewportRect.y != targetViewportY) {
      viewportRect.y += viewportYVel;

      if ((viewportYVel < 0 && viewportRect.y < targetViewportY) ||
          (viewportYVel > 0 && viewportRect.y > targetViewportY))
        viewportRect.y = targetViewportY;
    }

    if (targetCursorX != cursorRect.x) {
      cursorRect.x += cursorXVel;

      if ((cursorXVel < 0 && cursorRect.x < targetCursorX) ||
          (cursorXVel > 0 && cursorRect.x > targetCursorX))
        cursorRect.x = targetCursorX;
    }

    if (targetCursorY != cursorRect.y) {
      cursorRect.y += cursorYVel;

      if ((cursorYVel < 0 && cursorRect.y < targetCursorY) ||
          (cursorYVel > 0 && cursorRect.y > targetCursorY))
        cursorRect.y = targetCursorY;
    }

    cursorXVel = (targetCursorX - cursorRect.x) / 15.0;
    cursorYVel = (targetCursorY - cursorRect.y) / 15.0;
    if (cursorXVel > 0 && cursorXVel < 1)
      cursorXVel = 1;
    else if (cursorXVel < 0 && cursorXVel > -1)
      cursorXVel = -1;

    if (cursorYVel > 0 && cursorYVel < 1)
      cursorYVel = 1;
    else if (cursorYVel < 0 && cursorYVel > -1)
      cursorYVel = -1;

    viewportXVel = (targetViewportX - viewportRect.x) / 15.0;
    viewportYVel = (targetViewportY - viewportRect.y) / 15.0;

    if (viewportXVel > 0 && viewportXVel < 1)
      viewportXVel = 1;
    else if (viewportXVel < 0 && viewportXVel > -1)
      viewportXVel = -1;

    if (viewportYVel > 0 && viewportYVel < 1)
      viewportYVel = 1;
    else if (viewportYVel < 0 && viewportYVel > -1)
      viewportYVel = -1;
  }

  void pasteClipboard() {
    char *clipboard = SDL_GetClipboardText();

    Node *startPtr = linkedList->selected;

    for (int i = 0; clipboard[i] != '\0'; i++)
      linkedList->insert(clipboard[i]);

    linkedList->updatePosition(startPtr);
  }

  int devID = 0;

  void handleEvent(SDL_Event &event) {
    switch (event.type) {
    case SDL_QUIT:
      throw(-1);
      return;

    case SDL_TEXTINPUT:
      if (ctrlHeld)
        break;

      clickAudio();
      linkedList->insert(*event.text.text);
      linkedList->updatePosition(linkedList->selected->prev);
      break;

    case SDL_KEYUP:
      if (event.key.keysym.sym == SDLK_LCTRL)
        ctrlHeld = false;
      break;

    case SDL_KEYDOWN:
      if (event.key.keysym.scancode == 40) {
        linkedList->insert('\n');
        linkedList->updatePosition(linkedList->selected->prev);
      }

      switch (event.key.keysym.sym) {

      case SDLK_LCTRL:
        ctrlHeld = true;
        break;

      case SDLK_LEFT:
        ctrlHeld ? linkedList->ctrlLeft() : linkedList->selectLeft();
        break;

      case SDLK_RIGHT:
        ctrlHeld ? linkedList->ctrlRight() : linkedList->selectRight();
        break;

      case SDLK_BACKSPACE:
        ctrlHeld ? linkedList->ctrlBackspace() : linkedList->backspace();
        break;

      case SDLK_EQUALS:
        if (ctrlHeld)
          zoom(false);
        break;

      case SDLK_MINUS:
        if (ctrlHeld)
          zoom(true);
        break;

      case SDLK_v:
        if (ctrlHeld)
          pasteClipboard();
        break;

      case SDLK_TAB:
        linkedList->tab();
        break;

      case SDLK_UP:
        linkedList->selectUp();
        break;

      case SDLK_DELETE:
        ctrlHeld ? linkedList->ctrlDel() : linkedList->del();
        break;

      case SDLK_DOWN:
        linkedList->selectDown();
        break;

      case SDLK_CAPSLOCK:
        capsLock = SDL_GetModState() & KMOD_CAPS;
        targetSwayX = targetSwayY = swayX = swayY = 0;
        break;
      }
    }

    setScale();
    centerCursor();
  }

  void mainLoop() {
    int x = 0;
    unsigned int currentTime = SDL_GetTicks64();
    unsigned int lastTime = currentTime;

    while (1) {
      currentTime = SDL_GetTicks64();
      if (currentTime < lastTime + (1000 / 60))
        continue;

      lastTime = currentTime;

      if (capsLock)
        shakeScreen();
      else
        sway();

      applyScale();

      smoothTransitionCursor();
      setSway();

      SDL_RenderClear(renderer);

      for (Node *ptr = linkedList->head; ptr != nullptr; ptr = ptr->next) {
        SDL_RenderCopy(renderer, ptr->data->texture, NULL, ptr->data->rect);

        if (ptr == linkedList->selected) {
          SDL_SetRenderDrawColor(renderer, Color::White.r, Color::White.g,
                                 Color::White.b, 255);
          SDL_RenderFillRect(renderer, &cursorRect);
          SDL_SetRenderDrawColor(renderer, Color::Black.r, Color::Black.g,
                                 Color::Black.b, 0);
        }
      }

      SDL_RenderSetScale(renderer, scale, scale);
      SDL_RenderSetViewport(renderer, &swayRect);
      SDL_RenderPresent(renderer);

      SDL_Event event;
      if (SDL_PollEvent(&event))
        try {
          handleEvent(event);
        } catch (int) {
          break;
        }
    }
  }
};