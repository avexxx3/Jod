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
    // startMusic();
    // setMusicScale();
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

  void resetViewport() {
    // viewportRect.x = viewportRect.y = targetX = targetY = 10;
  }

  void setScale() {
    if (maxScaleReached || zoomed)
      return;

    Node *ptr = linkedList->head;

    double newScale = 0;
    double minScale = 0.75;

    while (ptr->next) {
      if (ptr->next->data->ch == '\n' || !ptr->next->next) {
        newScale = (1260.0 / 2) / (ptr->data->rect->x + (10 - targetSwayX));

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

  /*
  // Uint64 musicStartTime;
  // std::unordered_map<int, double> analyzeVolume(const std::string &path) {
  //   SF_INFO sfinfo;
  //   SNDFILE *file = sf_open(path.c_str(), SFM_READ, &sfinfo);
  //   if (!file) {
  //     std::cerr << "Error opening file: " << sf_strerror(file) <<
  std::endl;
  //     return {};
  //   }

  //   std::unordered_map<int, double> volume_data;
  //   double duration = static_cast<double>(sfinfo.frames) /
  sfinfo.samplerate;

  //   double samples_per_second =
  //       static_cast<double>(sfinfo.channels * sfinfo.samplerate);

  //   double samples_per_interval =
  //       samples_per_second * 0.5; // iterate over 0.5 second intervals

  //   int64_t samples = static_cast<int64_t>(
  //       std::ceil(duration * 2.0)); // count in 0.5 second intervals

  //   for (int64_t i = 0; i < samples; ++i) {
  //     double sum = 0.0;
  //     int count = 0;
  //     std::vector<float> buffer(
  //         static_cast<size_t>(samples_per_interval * sfinfo.channels));

  //     int frames_read = sf_readf_float(
  //         file, buffer.data(), static_cast<size_t>(samples_per_interval));
  //     count = frames_read * sfinfo.channels;

  //     for (int j = 0; j < count; ++j) {
  //       sum += std::fabs(buffer[j]);
  //     }

  //     if (count > 0) {
  //       float volume = static_cast<float>(sum / count);
  //       volume_data.insert({i / 2.0, volume});
  //     } else {
  //       break;
  //     }
  //   }

  //   // Normalize to 1 to 1.2
  //   float max_volume = 0.0;
  //   for (const auto &item : volume_data) {
  //     if (item.second > max_volume) {
  //       max_volume = item.second;
  //     }
  //   }

  //   for (auto &item : volume_data) {
  //     item.second = 1.0 + (item.second / max_volume); // Scale to 1 to 1.2
  //   }

  //   sf_close(file);
  //   return volume_data;
  // }

  // std::unordered_map<int, double> fairyFountain = {
  //     {0.0, 1.0539618},   {1.0, 1.0191282},   {2.0, 1.0469373},
  //     {3.0, 1.0462487},   {4.0, 1.0750084},   {5.0, 1.0065316},
  //     {6.0, 1.0042076},   {7.0, 1.0522273},   {8.0, 1.025394},
  //     {9.0, 1.034033},    {10.0, 1.0470625},  {11.0, 1.0846603},
  //     {12.0, 1.0299637},  {13.0, 1.0302801},  {14.0, 1.0577453},
  //     {15.0, 1.0187851},  {16.0, 1.0187684},  {17.0, 1.0613217},
  //     {18.0, 1.0678097},  {19.0, 1.0670584},  {20.0, 1.044165},
  //     {21.0, 1.0506502},  {22.0, 1.020525},   {23.0, 1.0273547},
  //     {24.0, 1.0608515},  {25.0, 1.0589116},  {26.0, 1.0522001},
  //     {27.0, 1.0559226},  {28.0, 1.0570222},  {29.0, 1.0223359},
  //     {30.0, 1.01924},    {31.0, 1.0606316},  {32.0, 1.0643392},
  //     {33.0, 1.0389143},  {34.0, 1.0070635},  {35.0, 1.0236523},
  //     {36.0, 1.0398227},  {37.0, 1.0254357},  {38.0, 1.0549189},
  //     {39.0, 1.0388132},  {40.0, 1.0759989},  {41.0, 1.0293479},
  //     {42.0, 1.0308719},  {43.0, 1.0543418},  {44.0, 1.0195435},
  //     {45.0, 1.0461177},  {46.0, 1.0307769},  {47.0, 1.0958331},
  //     {48.0, 1.041738},   {49.0, 1.0493251},  {50.0, 1.0526147},
  //     {51.0, 1.0236896},  {52.0, 1.0390139},  {53.0, 1.0509338},
  //     {54.0, 1.084575},   {55.0, 1.0264554},  {56.0, 1.0565127},
  //     {57.0, 1.0573399},  {58.0, 1.0198699},  {59.0, 1.0212908},
  //     {60.0, 1.0684253},  {61.0, 1.0725185},  {62.0, 1.0219313},
  //     {63.0, 1.0091562},  {64.0, 1.0479617},  {65.0, 1.0248789},
  //     {66.0, 1.026599},   {67.0, 1.0587901},  {68.0, 1.0602707},
  //     {69.0, 1.054426},   {70.0, 1.0304669},  {71.0, 1.0573671},
  //     {72.0, 1.0238066},  {73.0, 1.0212786},  {74.0, 1.0583516},
  //     {75.0, 1.052544},   {76.0, 1.089848},   {77.0, 1.0211016},
  //     {78.0, 1.0615649},  {79.0, 1.0356197},  {80.0, 1.0270627},
  //     {81.0, 1.0460335},  {82.0, 1.038085},   {83.0, 1.0436254},
  //     {84.0, 1.0526226},  {85.0, 1.0320243},  {86.0, 1.0541759},
  //     {87.0, 1.020741},   {88.0, 1.0536994},  {89.0, 1.0461335},
  //     {90.0, 1.0746785},  {91.0, 1.0111359},  {92.0, 1.0113709},
  //     {93.0, 1.0540451},  {94.0, 1.0262016},  {95.0, 1.0409887},
  //     {96.0, 1.0469328},  {97.0, 1.085216},   {98.0, 1.0305254},
  //     {99.0, 1.0320295},  {100.0, 1.0580842}, {101.0, 1.0197448},
  //     {102.0, 1.0237021}, {103.0, 1.0630654}, {104.0, 1.0726595},
  //     {105.0, 1.0644488}, {106.0, 1.0472598}, {107.0, 1.0516446},
  //     {108.0, 1.0213132}, {109.0, 1.0281681}, {110.0, 1.0614359},
  //     {111.0, 1.0619695}, {112.0, 1.0497994}, {113.0, 1.0571592},
  //     {114.0, 1.0576891}, {115.0, 1.0221752}, {116.0, 1.0214162},
  //     {117.0, 1.0628933}, {118.0, 1.0665034}, {119.0, 1.0372632},
  //     {120.0, 1.0113314}, {121.0, 1.0310806}, {122.0, 1.036843},
  //     {123.0, 1.0259457}, {124.0, 1.0568092}, {125.0, 1.0430595},
  //     {126.0, 1.0720007}, {127.0, 1.0296885}, {128.0, 1.0343142},
  //     {129.0, 1.0512575}, {130.0, 1.0200709}, {131.0, 1.049244},
  //     {132.0, 1.0324748}, {133.0, 1.0976923}, {134.0, 1.0400252},
  //     {135.0, 1.0504198}, {136.0, 1.0533599}, {137.0, 1.0248957},
  //     {138.0, 1.0362841}, {139.0, 1.0384847}, {140.0, 1.0511881},
  //     {141.0, 1.0288624}, {142.0, 1.056126},  {143.0, 1.0585124},
  //     {144.0, 1.0216776}, {145.0, 1.0336574}, {146.0, 1.0706453},
  //     {147.0, 1.0744333}, {148.0, 1.0235084}, {149.0, 1.0161121},
  //     {150.0, 1.0498129}, {151.0, 1.0270624}, {152.0, 1.0319545},
  //     {153.0, 1.0630705}, {154.0, 1.0654249}, {155.0, 1.0514996},
  //     {156.0, 1.0326604}, {157.0, 1.041739},  {158.0, 1.0239855},
  //     {159.0, 1.0267135}, {160.0, 1.054354},  {161.0, 1.0366372},
  //     {162.0, 1.0503742}, {163.0, 1.0196366}, {164.0, 1.0366793},
  //     {165.0, 1.028129},  {166.0, 1.0279019}, {167.0, 1.0597757},
  //     {168.0, 1.0441166}, {169.0, 1.0452954}, {170.0, 1.0374845},
  //     {171.0, 1.0344762}, {172.0, 1.0367169}, {173.0, 1.0219887},
  //     {174.0, 1.0559242}, {175.0, 1.048431},  {176.0, 1.0564508},
  //     {177.0, 1.0129397}, {178.0, 1.0161691}, {179.0, 1.0364211},
  //     {180.0, 1.0279181}, {181.0, 1.0498234}, {182.0, 1.0407522},
  //     {183.0, 1.0524461}, {184.0, 1.0302489}, {185.0, 1.0341183},
  //     {186.0, 1.0424707}, {187.0, 1.0225754}, {188.0, 1.0379472},
  //     {189.0, 1.0549978}, {190.0, 1.0451473}, {191.0, 1.0364207},
  //     {192.0, 1.0301095}, {193.0, 1.033666},  {194.0, 1.024426},
  //     {195.0, 1.04117},   {196.0, 1.0529711}, {197.0, 1.0462009},
  //     {198.0, 1.0358698}, {199.0, 1.0436862}, {200.0, 1.0381925},
  //     {201.0, 1.0266929}, {202.0, 1.0325817}, {203.0, 1.064037},
  //     {204.0, 1.0495063}, {205.0, 1.0400103}, {206.0, 1.0179399},
  //     {207.0, 1.025682},  {208.0, 1.0337622}, {209.0, 1.0269544},
  //     {210.0, 1.070808},  {211.0, 1.0442847}, {212.0, 1.0464779},
  //     {213.0, 1.0329504}, {214.0, 1.0423115}, {215.0, 1.0944153},
  //     {216.0, 1.0914478}, {217.0, 1.0961837}, {218.0, 1.0415926},
  //     {219.0, 1.0964963}, {220.0, 1.0374297}, {221.0, 1.0506428},
  //     {222.0, 1.0534897}, {223.0, 1.0248623}, {224.0, 1.0457293},
  //     {225.0, 1.0462224}, {226.0, 1.0854757}, {227.0, 1.0316309},
  //     {228.0, 1.0532193}, {229.0, 1.0585724}, {230.0, 1.0216675},
  //     {231.0, 1.0312673}, {232.0, 1.0672807}, {233.0, 1.074142},
  //     {234.0, 1.0202196}, {235.0, 1.0123593}, {236.0, 1.0497097},
  //     {237.0, 1.0257933}, {238.0, 1.0288396}, {239.0, 1.0612541},
  //     {240.0, 1.0676802}, {241.0, 1.0482036}, {242.0, 1.0315807},
  //     {243.0, 1.0586898}, {244.0, 1.0230699}, {245.0, 1.0238614},
  //     {246.0, 1.0627657}, {247.0, 1.0550417}, {248.0, 1.0889877},
  //     {249.0, 1.0259455}, {250.0, 1.063111},  {251.0, 1.0317358},
  //     {252.0, 1.0278441}, {253.0, 1.0477948}, {254.0, 1.0401821},
  //     {255.0, 1.0417757}, {256.0, 1.0547162}, {257.0, 1.0366892},
  //     {258.0, 1.0492089}, {259.0, 1.0208298}, {260.0, 1.0597227},
  //     {261.0, 1.0486714}, {262.0, 1.069068},  {263.0, 1.0110189},
  //     {264.0, 1.0102859}, {265.0, 1.0549064}, {266.0, 1.026216},
  //     {267.0, 1.045633},  {268.0, 1.0404546}, {269.0, 1.086218},
  //     {270.0, 1.0296763}, {271.0, 1.0315192}, {272.0, 1.0573561},
  //     {273.0, 1.0199214}, {274.0, 1.0287375}, {275.0, 1.0564191},
  //     {276.0, 1.0781527}, {277.0, 1.0596077}, {278.0, 1.0488703},
  //     {279.0, 1.0521291}, {280.0, 1.0219148}, {281.0, 1.0291125},
  //     {282.0, 1.0624459}, {283.0, 1.0669972}, {284.0, 1.0444957},
  //     {285.0, 1.0575329}, {286.0, 1.0577528}, {287.0, 1.0219016},
  //     {288.0, 1.0218234}, {289.0, 1.0639112}, {290.0, 1.0663844},
  //     {291.0, 1.0361103}, {292.0, 1.0109417}, {293.0, 1.0366086},
  //     {294.0, 1.0327845}, {295.0, 1.0258089}, {296.0, 1.0574294},
  //     {297.0, 1.0488575}, {298.0, 1.0661819}, {299.0, 1.0299342},
  //     {300.0, 1.0391452}, {301.0, 1.0457077}, {302.0, 1.0198809},
  //     {303.0, 1.0520964}, {304.0, 1.0343667}, {305.0, 1.0985506},
  //     {306.0, 1.0315578}, {307.0, 1.0503452}, {308.0, 1.0536344},
  //     {309.0, 1.0251626}, {310.0, 1.0360218}, {311.0, 1.0356474},
  //     {312.0, 1.0505242}, {313.0, 1.0331378}, {314.0, 1.0492098},
  //     {315.0, 1.0565581}, {316.0, 1.0196108}, {317.0, 1.0277448},
  //     {318.0, 1.0618345}, {319.0, 1.074433},  {320.0, 1.0166757},
  //     {321.0, 1.0092764}, {322.0, 1.049084},  {323.0, 1.0250919},
  //     {324.0, 1.0246006}, {325.0, 1.0585544}, {326.0, 1.0673106},
  //     {327.0, 1.0474589}, {328.0, 1.0307158}, {329.0, 1.058762},
  //     {330.0, 1.0214791}, {331.0, 1.0209839}, {332.0, 1.0591412},
  //     {333.0, 1.0530683}, {334.0, 1.0883417}, {335.0, 1.0285242},
  //     {336.0, 1.060272},  {337.0, 1.028542},  {338.0, 1.0277587},
  //     {339.0, 1.0596027}, {340.0, 1.0511205}, {341.0, 1.0623413},
  //     {342.0, 1.0547528}, {343.0, 1.0370047}, {344.0, 1.0461742},
  //     {345.0, 1.0203713}, {346.0, 1.0555999}, {347.0, 1.0472184},
  //     {348.0, 1.0669035}, {349.0, 1.0098113}, {350.0, 1.0087172},
  //     {351.0, 1.0546379}, {352.0, 1.0261202}, {353.0, 1.0477146},
  //     {354.0, 1.0351549}, {355.0, 1.086605},  {356.0, 1.029256},
  //     {357.0, 1.0310224}, {358.0, 1.0566306}, {359.0, 1.0189722},
  //     {360.0, 1.0291574}, {361.0, 1.0518608}, {362.0, 1.0810722},
  //     {363.0, 1.0557581}, {364.0, 1.0477222}, {365.0, 1.0515093},
  //     {366.0, 1.0211465}, {367.0, 1.0276151}, {368.0, 1.0445832},
  //     {369.0, 1.0462166}, {370.0, 1.0301242}, {371.0, 1.0573506},
  //     {372.0, 1.0575641}, {373.0, 1.0208071}, {374.0, 1.0198263},
  //     {375.0, 1.0634528}, {376.0, 1.0662066}, {377.0, 1.0345116},
  //     {378.0, 1.0094934}, {379.0, 1.0383193}, {380.0, 1.0292766},
  //     {381.0, 1.0253394}, {382.0, 1.0567424}, {383.0, 1.0503552},
  //     {384.0, 1.0638384}, {385.0, 1.0298271}, {386.0, 1.0407159},
  //     {387.0, 1.0435759}, {388.0, 1.0194361}, {389.0, 1.0522873},
  //     {390.0, 1.0357938}, {391.0, 1.1},       {392.0, 1.0264192},
  //     {393.0, 1.0501528}, {394.0, 1.0530916}, {395.0, 1.0248663},
  //     {396.0, 1.0507557}, {397.0, 1.0379553}, {398.0, 1.0845723},
  //     {399.0, 1.0331548}, {400.0, 1.0484456}, {401.0, 1.0090423},
  //     {402.0, 1.0052091}, {403.0, 1.0106392}, {404.0, 1.0082655},
  //     {405.0, 1.0059555}, {406.0, 1.0069368}, {407.0, 1.0058316},
  //     {408.0, 1.0043783}, {409.0, 1.0049368}, {410.0, 1.012081},
  //     {411.0, 1.0111},    {412.0, 1.0079705}, {413.0, 1.007832},
  //     {414.0, 1.0071062}, {415.0, 1.0041038}, {416.0, 1.0057083},
  //     {417.0, 1.0121309}, {418.0, 1.0144058}, {419.0, 1.0082046},
  //     {420.0, 1.0085423}, {421.0, 1.0084304}, {422.0, 1.0057251},
  //     {423.0, 1.0060164}, {424.0, 1.0113658}, {425.0, 1.0272732},
  //     {426.0, 1.0151265}, {427.0, 1.0130787}, {428.0, 1.0119606},
  //     {429.0, 1.0142634}, {430.0, 1.01686},   {431.0, 1.0090681},
  //     {432.0, 1.0445287}, {433.0, 1.0239687}, {434.0, 1.0134758},
  //     {435.0, 1.009496},  {436.0, 1.0133628}, {437.0, 1.0114979},
  //     {438.0, 1.0086277}, {439.0, 1.0292784}, {440.0, 1.0181499},
  //     {441.0, 1.0090307}, {442.0, 1.0061063}, {443.0, 1.0095248},
  //     {444.0, 1.0059764}, {445.0, 1.0060604}, {446.0, 1.0147847},
  //     {447.0, 1.0113704}, {448.0, 1.0051763}, {449.0, 1.0025986},
  //     {450.0, 1.0047125}, {451.0, 1.0019908}, {452.0, 1.0023061},
  //     {453.0, 1.0032967}, {454.0, 1.002495},  {455.0, 1.0004749},
  //     {456.0, 1.0000861},
  // };
  // void startMusic() {
  //   beatMap = analyzeVolume("res/fountain.wav");
  //   SDL_QueueAudio(deviceId, wav_buffer, wav_length);
  //   SDL_PauseAudioDevice(deviceId, 0);
  //   musicStartTime = SDL_GetTicks64();
  // }
  // double musicScale = 1;
  // double targetMusicScale = 1;
  // double musicScaleSpeed = 0;
  // int previousTime = 0;
  // bool scaleOut = true;
  // void setMusicScale() {
  //   int currentTime = (SDL_GetTicks64() - musicStartTime) / 500;

  //   if (currentTime == previousTime)
  //     return;

  //   previousTime = currentTime;
  //   scaleOut = true;

  //   try {
  //     targetMusicScale = 1 + (fairyFountain.at(currentTime) - 1) / 2.0;
  //   } catch (const std::out_of_range &e) {
  //     targetMusicScale = musicScale = 1;
  //   }

  //   musicScaleSpeed = (targetMusicScale - musicScale) / 15.0;
  // }
  // void applyMusicScale() {
  //   musicScale += musicScaleSpeed;
  //   if ((musicScaleSpeed < 0 && musicScale < targetMusicScale) ||
  //       (musicScaleSpeed > 0 && musicScale > targetMusicScale)) {
  //     musicScale = targetMusicScale;
  //     scaleOut = false;
  //   }
  // }
  // void reverseMusicScale() {
  //   if (musicScale == 1)
  //     return;

  //   musicScale -= musicScaleSpeed;
  //   if ((musicScaleSpeed < 0 && musicScale > 1) ||
  //       (musicScaleSpeed > 0 && musicScale < 1)) {
  //     musicScale = 1;
  //     std::cout << previousTime << '\n';
  //   }
  // }
*/
};