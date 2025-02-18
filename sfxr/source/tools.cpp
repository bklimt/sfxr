/*
   Copyright (c) 2007 Tomas Pettersson <drpetter@gmail.com>

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

#include "tools.h"

#include "sdlkit.h"

int LoadTGA(Spriteset& tiles, const char* filename) {
  FILE* file;
  unsigned char byte, crap[16], id_length;
  int i, width, height, channels, x, y;
  file = fopen(filename, "rb");
  if (!file) return -1;

  int n;
  (void)n;
  n = fread(&id_length, 1, 1, file);
  n = fread(crap, 1, 11, file);
  width = 0;
  height = 0;
  n = fread(&width, 1, 2, file);   // width
  n = fread(&height, 1, 2, file);  // height
  n = fread(&byte, 1, 1, file);    // bits
  channels = byte / 8;
  (void)channels;
  n = fread(&byte, 1, 1, file);  // image descriptor byte (per-bit info)
  for (i = 0; i < id_length; i++)
    n = fread(&byte, 1, 1, file);  // image description
  tiles.data = (DWORD*)malloc(width * height * sizeof(DWORD));
  for (y = height - 1; y >= 0; y--)
    for (x = 0; x < width; x++) {
      DWORD pixel = 0;
      n = fread(&byte, 1, 1, file);
      pixel |= byte;
      n = fread(&byte, 1, 1, file);
      pixel |= byte << 8;
      n = fread(&byte, 1, 1, file);
      pixel |= byte << 16;
      tiles.data[y * width + x] = pixel;
    }
  fclose(file);
  tiles.height = height;
  tiles.width = height;
  tiles.pitch = width;

  return 0;
}

void ClearScreen(DWORD color) {
  for (int y = 0; y < 480 * UI_SCALE; y++) {
    int offset = y * ddkpitch;
    for (int x = 0; x < 640 * UI_SCALE; x += 8) {
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
      ddkscreen32[offset++] = color;
    }
  }
}

void DrawBar(int sx, int sy, int w, int h, DWORD color) {
  sx *= UI_SCALE;
  sy *= UI_SCALE;
  w *= UI_SCALE;
  h *= UI_SCALE;
  for (int y = sy; y < sy + h; y++) {
    int offset = y * ddkpitch + sx;
    int x1 = 0;
    if (w > 8) {
      for (x1 = 0; x1 < w - 8; x1 += 8) {
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
        ddkscreen32[offset++] = color;
      }
    }
    for (int x = x1; x < w; x++) {
      ddkscreen32[offset++] = color;
    }
  }
}

void DrawBox(int sx, int sy, int w, int h, DWORD color) {
  DrawBar(sx, sy, w, 1, color);
  DrawBar(sx, sy, 1, h, color);
  DrawBar(sx + w, sy, 1, h, color);
  DrawBar(sx, sy + h, w + 1, 1, color);
}

void DrawSprite(Spriteset& sprites, int sx, int sy, int i, DWORD color) {
  sx *= UI_SCALE;
  sy *= UI_SCALE;
  for (int y = 0; y < sprites.height; y++) {
    for (int y_scale = 0; y_scale < UI_SCALE; y_scale++) {
      int offset = (sy + (y * UI_SCALE) + y_scale) * ddkpitch + sx;
      int spoffset = y * sprites.pitch + i * sprites.width;
      if (color & 0xFF000000) {
        for (int x = 0; x < sprites.width; x++) {
          DWORD p = sprites.data[spoffset++];
          for (int x_scale = 0; x_scale < UI_SCALE; x_scale++) {
            if (p != 0x300030) {
              ddkscreen32[offset + (x * UI_SCALE) + x_scale] = p;
            }
          }
        }
      } else {
        for (int x = 0; x < sprites.width; x++) {
          DWORD p = sprites.data[spoffset++];
          for (int x_scale = 0; x_scale < UI_SCALE; x_scale++) {
            if (p != 0x300030) {
              ddkscreen32[offset + (x * UI_SCALE) + x_scale] = color;
            }
          }
        }
      }
    }
  }
}

void DrawText(Spriteset& font, int sx, int sy, DWORD color, const char* string,
              ...) {
  char string2[256];
  va_list args;

  va_start(args, string);
  vsprintf(string2, string, args);
  va_end(args);

  int len = strlen(string2);
  for (int i = 0; i < len; i++)
    DrawSprite(font, sx + i * 8, sy, string2[i] - ' ', color);
}

bool MouseInBox(int x, int y, int w, int h) {
  x *= UI_SCALE;
  y *= UI_SCALE;
  w *= UI_SCALE;
  h *= UI_SCALE;
  if (mouse_x >= x && mouse_x < x + w && mouse_y >= y && mouse_y < y + h)
    return true;
  return false;
}
