/*
   Copyright (c) 2007 mjau/GerryJJ

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "sdlkit.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <sys/stat.h>
#include <list>
#include <string>
#include <iostream>

#ifdef WIN32
#include <filesystem>
#include <locale>
#include <codecvt>
#else
#include <dirent.h>
#include <unistd.h>
#endif


void error(const char* file, unsigned int line, const char* msg) {
  fprintf(stderr, "[!] %s:%u  %s\n", file, line, msg);
  exit(1);
}

bool keys[SDL_NUM_SCANCODES];

bool DPInput::KeyPressed(SDL_Keycode key) {
  bool r = keys[key];
  keys[key] = false;
  return r;
}

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* texture = nullptr;
Uint32* ddkscreen32;
Uint16* ddkscreen16;
int ddkpitch;
int mouse_x, mouse_y, mouse_px, mouse_py;
bool mouse_left = false, mouse_right = false, mouse_middle = false;
bool mouse_leftclick = false, mouse_rightclick = false,
     mouse_middleclick = false;

SDL_Surface* sdlscreen = NULL;

void updateMouse(const SDL_MouseButtonEvent& event) {
  mouse_px = mouse_x;
  mouse_py = mouse_y;
  mouse_x = event.x;
  mouse_y = event.y;
  /*
  if (renderer != nullptr) {
    float fmouse_x;
    float fmouse_y;
    SDL_RenderWindowToLogical(renderer, mouse_x, mouse_y, &fmouse_x, &fmouse_y);
    mouse_x = fmouse_x;
    mouse_y = fmouse_y;
  }
  */
  bool mouse_left_p = mouse_left;
  bool mouse_right_p = mouse_right;
  bool mouse_middle_p = mouse_middle;
  mouse_left = event.button == SDL_BUTTON_LEFT && event.state == SDL_PRESSED;
  mouse_right = event.button == SDL_BUTTON_RIGHT && event.state == SDL_PRESSED;
  mouse_middle =
      event.button == SDL_BUTTON_MIDDLE && event.state == SDL_PRESSED;
  mouse_leftclick = mouse_left && !mouse_left_p;
  mouse_rightclick = mouse_right && !mouse_right_p;
  mouse_middleclick = mouse_middle && !mouse_middle_p;
  /*
  if (mouse_leftclick) {
    std::cout << "left click!" << std::endl;
  }
  if (mouse_rightclick) {
    std::cout << "right click!" << std::endl;
  }
  if (mouse_middleclick) {
    std::cout << "middle click!" << std::endl;
  }
  */
}

void sdlupdate() {
  mouse_px = mouse_x;
  mouse_py = mouse_y;
  Uint8 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
  if (renderer != nullptr) {
    float fmouse_x;
    float fmouse_y;
    SDL_RenderWindowToLogical(renderer, mouse_x, mouse_y, &fmouse_x, &fmouse_y);
    mouse_x = fmouse_x;
    mouse_y = fmouse_y;
  }
  bool mouse_left_p = mouse_left;
  bool mouse_right_p = mouse_right;
  bool mouse_middle_p = mouse_middle;
  mouse_left = buttons & SDL_BUTTON(1);
  mouse_right = buttons & SDL_BUTTON(3);
  mouse_middle = buttons & SDL_BUTTON(2);
  mouse_leftclick = mouse_left && !mouse_left_p;
  mouse_rightclick = mouse_right && !mouse_right_p;
  mouse_middleclick = mouse_middle && !mouse_middle_p;
  /*
  if (mouse_leftclick) {
    std::cout << "left click!" << std::endl;
  }
  if (mouse_rightclick) {
    std::cout << "right click!" << std::endl;
  }
  if (mouse_middleclick) {
    std::cout << "middle click!" << std::endl;
  }
  */
}

bool ddkLock() {
  if (SDL_MUSTLOCK(sdlscreen)) {
    if (SDL_LockSurface(sdlscreen) < 0) return false;
  }
  ddkpitch = sdlscreen->pitch / (sdlscreen->format->BitsPerPixel == 32 ? 4 : 2);
  ddkscreen16 = (Uint16*)(sdlscreen->pixels);
  ddkscreen32 = (Uint32*)(sdlscreen->pixels);
  return true;
}

void ddkUnlock() {
  if (SDL_MUSTLOCK(sdlscreen)) {
    SDL_UnlockSurface(sdlscreen);
  }
}

void ddkSetMode(int width, int height, int bpp, int refreshrate, int fullscreen,
                const char* title) {
  // VERIFY(sdlscreen = SDL_SetVideoMode(width, height, bpp,
  //                                     fullscreen ? SDL_FULLSCREEN : 0));
  // SDL_WM_SetCaption(title, title);

  uint32_t r_mask = 0x00FF0000;
  uint32_t g_mask = 0x0000FF00;
  uint32_t b_mask = 0x000000FF;
  uint32_t a_mask = 0xFF000000;
  VERIFY(sdlscreen = SDL_CreateRGBSurface(0, width, height, bpp, r_mask, g_mask,
                                          b_mask, a_mask));

  VERIFY(texture =
             SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888,
                               SDL_TEXTUREACCESS_STREAMING, width, height));

  VERIFY(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) == 0);
}

void flip() {
  if (SDL_UpdateTexture(texture, nullptr, sdlscreen->pixels,
                        sdlscreen->pitch) != 0) {
    std::cerr << "Unable to update particle texture" << std::endl;
  }
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0x33);
  if (SDL_RenderClear(renderer) != 0) {
    std::cerr << "Unable to clear: " << SDL_GetError() << std::endl;
  }
  if (SDL_RenderCopy(renderer, texture, nullptr, nullptr) != 0) {
    std::cerr << "Unable to copy particle texture: " << SDL_GetError()
              << std::endl;
  }
  SDL_RenderPresent(renderer);
}

bool Button(int x, int y, bool highlight, const char* text, int id);

#ifdef WIN32

std::string filename(const std::filesystem::path& path) {
  using convert_type = std::codecvt_utf8<wchar_t>;
  std::wstring_convert<convert_type, wchar_t> converter;
  std::wstring ws = path.filename().native();
  return converter.to_bytes(ws);
}

std::list<std::string> ioList(const std::string& dirname, bool directories,
                              bool files) {
  std::list<std::string> dirList;
  std::list<std::string> fileList;

  std::filesystem::path path(dirname);
  for (auto entry: std::filesystem::directory_iterator(path)) {
      if (entry.is_directory()) {
          if (directories) {
            dirList.push_back(filename(entry.path()));
          }
    } else if (files) {
        fileList.push_back(filename(entry.path()));
    }
  }

  dirList.sort();
  fileList.sort();

  fileList.splice(fileList.begin(), dirList);

  return fileList;
}

bool ioExists(const std::string& filename) {
  return std::filesystem::exists(std::filesystem::path(filename));
}

#else

bool ioIsDir(const std::string& filename) {
  using namespace std;
  struct stat status;
  stat(filename.c_str(), &status);

  return (status.st_mode & S_IFDIR);
}

std::list<std::string> ioList(const std::string& dirname, bool directories,
                              bool files) {
  using namespace std;
  list<string> dirList;
  list<string> fileList;

  DIR* dir = opendir(dirname.c_str());
  dirent* entry;

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_DIR) {
      if (directories) {
        dirList.push_back(entry->d_name);
      }
    } else if (files) {
      fileList.push_back(entry->d_name);
    }
  }

  closedir(dir);

  dirList.sort();
  fileList.sort();

  fileList.splice(fileList.begin(), dirList);

  return fileList;
}

bool ioExists(const std::string& filename) {
  return (access(filename.c_str(), 0) == 0);
}

#endif

extern DPInput* input;

bool file_select_update() {
  input->Update();  // (for keyboard input)

  // keydown=false;

  return true;
}

std::string stoupper(const std::string& s) {
  std::string result = s;
  std::string::iterator i = result.begin();
  std::string::iterator end = result.end();

  while (i != end) {
    *i = std::toupper((unsigned char)*i);
    ++i;
  }
  return result;
}

bool ioNew(const std::string& filename, bool readable, bool writeable) {
  if (ioExists(filename)) return false;

  FILE* file = fopen(filename.c_str(), "wb");
  if (file == NULL) return false;
  fclose(file);
  return true;
}

void ClearScreen(DWORD color);
extern int vcurbutton;

#include "tools.h"

extern Spriteset font;

std::string new_file(const std::string& forced_extension) {
  using namespace std;
  // SDL_EnableUNICODE(1);
  // SDL_EnableKeyRepeat(0, 0);

  string result;

  bool done = false;
  SDL_Event e;
  while (!done) {
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        case SDL_QUIT:
          exit(0);

        case SDL_KEYDOWN:
          if (e.key.keysym.sym == SDLK_ESCAPE) {
            return "";
          }
          if (e.key.keysym.sym == SDLK_RETURN) {
            done = true;
            break;
          }

          {
            char c = e.key.keysym.sym;
            if (0x21 <= c && c <= 0x7E) result += c;
          }

        default:
          break;
      }
    }
    sdlupdate();

    ClearScreen(0xC0B090);

    DrawText(font, 90, 150, 0x000000, "TYPE NEW FILE NAME:");
    DrawText(font, 100, 200, 0x000000, "%s", stoupper(result).c_str());

    SDL_Delay(5);

    // SDL_Flip(sdlscreen);
    flip();
  }

  // SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  // SDL_EnableUNICODE(0);

  // if(result.size() == 0)
  //	throw runtime_error("New file name is empty string.");

  if (result.size() < 6 ||
      result.substr(result.size() - 1 - 4, string::npos) != forced_extension)
    result += forced_extension;

  return result;
}

void DrawFileSelectScreen(std::list<std::string>& files, char* buf,
                          bool& gotFile, bool& done, bool showNewButton) {
  using namespace std;

  ddkLock();

  ClearScreen(0xC0B090);

  int i = 0, j = 0;
  for (list<string>::iterator e = files.begin(); e != files.end(); e++) {
    if (40 + 20 * i > sdlscreen->h - 50) {
      j++;
      i = 0;
    }
    if (Button(30 + 150 * j, 40 + 20 * i, false, stoupper(*e).c_str(),
               31 + i + j)) {
      gotFile = true;
      sprintf(buf, "%s", e->c_str());
      done = true;
    }
    i++;
  }

  if (Button(10, 10, false, "CANCEL", 400)) {
    gotFile = false;
    done = true;
  }

  if (showNewButton && Button(120, 10, false, "NEW FILE", 401)) {
    string s = new_file(".sfxr");
    if (s != "") {
      ioNew(s, true, true);
      files = ioList(".", false, true);

      for (list<string>::iterator e = files.begin(); e != files.end();) {
        if (e->find(".sfxr") == string::npos) {
          e = files.erase(e);
          continue;
        }

        e++;
      }
    }
  }

  ddkUnlock();

  if (!mouse_left) vcurbutton = -1;
}

bool select_file(char* buf, bool showNewButton) {
  // FIXME: Needs directory browsing

  bool gotFile = false;
  using namespace std;
  list<string> files;
  files = ioList(".", false, true);

  for (list<string>::iterator e = files.begin(); e != files.end();) {
    if (e->find(".sfxr") == string::npos) {
      e = files.erase(e);
      continue;
    }

    e++;
  }

  bool done = false;
  SDL_Event e;
  while (!done) {
    SDL_PollEvent(&e);
    switch (e.type) {
      case SDL_QUIT:
        exit(0);

      case SDL_KEYDOWN:
        keys[e.key.keysym.sym] = true;
        break;

      default:
        break;
    }
    sdlupdate();

    DrawFileSelectScreen(files, buf, gotFile, done, showNewButton);

    SDL_Delay(5);

    // SDL_Flip(sdlscreen);
    flip();
  }
  return gotFile;
}

void sdlquit() {
  ddkFree();
  if (sdlscreen != nullptr) {
    SDL_FreeSurface(sdlscreen);
  }
  if (texture != nullptr) {
    SDL_DestroyTexture(texture);
  }
  if (renderer != nullptr) {
    SDL_DestroyRenderer(renderer);
  }
  if (window != nullptr) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

int sdlinit() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    std::cerr << "Unable to init SDL: " << SDL_GetError() << std::endl;
    return -1;
  }
  atexit(sdlquit);

  bool fullscreen = false;
  window = SDL_CreateWindow(
      "sfxr", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640 * UI_SCALE,
      480 * UI_SCALE,
      SDL_WINDOW_RESIZABLE | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));

  if (window == nullptr) {
    std::cerr << "Unable to create window: " << SDL_GetError() << std::endl;
    return -1;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr) {
    std::cerr << "Unable to renderer: " << SDL_GetError() << std::endl;
    return -1;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
  SDL_RenderSetLogicalSize(renderer, 640 * UI_SCALE, 480 * UI_SCALE);

  SDL_Surface* icon;
  icon = SDL_LoadBMP("/usr/local/share/sfxr/images/sfxr.bmp");
  if (!icon) icon = SDL_LoadBMP("images/sfxr.bmp");
  // if (icon) SDL_WM_SetIcon(icon, NULL);

  memset(keys, 0, sizeof(keys));
  ddkInit();

  return 0;
}

void loop(void) {
  SDL_Event e;
  while (true) {
    SDL_PollEvent(&e);
    switch (e.type) {
      case SDL_QUIT:
        exit(0);

      case SDL_KEYDOWN:
        keys[e.key.keysym.sym] = true;
        break;

      case SDL_MOUSEBUTTONDOWN:
        // std::cout << "mouse button down" << std::endl;
        updateMouse(e.button);
        break;

      case SDL_MOUSEBUTTONUP:
        // std::cout << "mouse button up" << std::endl;
        updateMouse(e.button);
        break;

      default:
        break;
    }
    // sdlupdate();
    if (!ddkCalcFrame()) return;
    // SDL_Flip(sdlscreen);
    flip();
  }
}

int main(int argc, char* argv[]) {
  int n = sdlinit();
  if (n != 0) {
    return n;
  }
  loop();
  return 0;
}
