#pragma once
#include "basics.h"
#include "evolver.h"

class Covolver : public EvolverBase
{
public:
  int depth;
  int frame;
  int type;

  static const int numLevels = 8; // higher levels are higher resolution
  char *grids[numLevels][numLevels][numLevels];
  void Covolver()
  {
    for (int i = 0; i<numLevels; i++)
      for (int j = 0; j<numLevels; j++)
        for (int k = 0; k<numLevels; k++)
          grids[i][j] = new char[1<<(i + j + k)];
  }
  bool get(int levelX, int levelY, int levelZ, int x, int y, int z)
  {
    char *grid = grids[levelX][levelY][levelZ];
    return ((*(grid + x + (y<<levelX) + (z<<(levelX+levelY)))) & (frame&1 + 1))>0;
  }
  void set(int levelX, int levelY, int levelZ, int x, int y, int z, bool val)
  {
    char *grid = grids[levelX][levelY][levelZ];
    char &res = *(grid + x + (y<<levelX) + (z<<(levelX+levelY)));
    if (val)
      res = res | (2 - (frame&1));
    else
      res = res & ~(2 - (frame&1));
  }

  virtual void load(char* fileName, int type);
  virtual void draw();
  virtual void update();
  virtual void reset();
  virtual void set(const Evolver& evolver);
  virtual void write(FILE* fp);
};
