#pragma once
#include "basics.h"
#include "ScreenColour.h"

class Image
{
public:
  int width;
  int height;
  int widthShift;
  int *data;
  GLuint textureID;

  inline void setPixel(int x, int y, const int& colour)
  {
    int *ptr = data + x+1 + (y+1)*width;
    *ptr = colour;
  }
  inline int& pixel(int x, int y)
  {
    int *ptr = data + x+1 + (y+1)*width;
    return *ptr;
  }
  inline bool isSet(int x, int y)
  {
    int *ptr = data + x + 1 + (y + 1)*width;
    return (*ptr & 128) != 0;
  }
  inline bool isSet(int x, int y, bool centre)
  {
    if (x == -1 || y == -1 || x == width - 2 || y == height - 2)
      return centre;
    int *ptr = data + x + 1 + (y + 1)*width;
    return (*ptr & 128) != 0;
  }
  void draw();
  void clear(char shade = 0);
  void generateTexture();

  Image(int width, int height);
  ~Image(void);
};
