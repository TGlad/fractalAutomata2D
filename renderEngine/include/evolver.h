#pragma once
#include "basics.h"
#include "ScreenColour.h"
extern bool g_timeSymmetric;

class Evolver
{
public:
  // type 1,2
  int totalMasks[32];

  // type 3
  bool parentMasks[16];
  bool siblingMasks[512];
  bool childMasks[16];

  // type 6
  bool parentsAdd[6][3][3];
  bool parentsRemove[6][3][3];

  bool octagonalMasks[128];

  int depth;
  int frame;
  int type;
  class Image** bitmaps;
  class Image** bitmapDuals;

  Evolver(int depth);
  void load(char* fileName, int type);
  void randomiseMasks(const Evolver& master, float percentVariation);
  void randomise(bool *starts = NULL);
  void checkSiblings();
  bool getNewValueSimple(int level, int X, int Y);
  bool getNewValue(int level, int X, int Y);
  bool getNewValue2(int level, int X, int Y);
  bool getNewValueAllSymmetries(int level, int X, int Y);
  bool getNewValueChapter6(int level, int X, int Y);
  bool getNewValueParentsOnly(int level, int X, int Y);
  bool getNewValue4Parents(int level, int X, int Y);
  bool getNewValueParentsOctagonal2(Image* image, int X, int Y, bool flip, bool extended, int level);
  bool checkAddRemove(int level, int X, int Y, bool addRemove[6][3][3], int i, bool flip);
  void draw();
  void drawMask();
  void update();
  void reset();
  void set(const Evolver& evolver);
  void read(FILE* fp);
  void write(FILE* fp);
  inline void adjustPixel(int& pixel, bool set)
  {
    if (g_timeSymmetric)
    {
      if (set)
      {
        if (pixel & 128)
          pixel |= 256;
        else
          pixel &= ~256;
      }
      else
      {
        if (pixel & 128)
          pixel &= ~256;
        else
          pixel |= 256;
      }
    }
    else if (set)
      pixel |= 256;
  }
};
