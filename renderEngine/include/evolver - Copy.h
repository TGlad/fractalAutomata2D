#pragma once
#include "basics.h"
#include "ScreenColour.h"

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

  // type 7
  float parentWeight;
  float siblingWeights[3][3];
  float childWeights[2][2];
  float totalWeight;
  float totalWeightNoChildren;
  float covolveScale;
  int parentW;
  int siblingWs[3][3];
  int childWs[2][2];


  int depth;
  int frame;
  class Image** bitmaps;

  Evolver(int depth);
  void randomiseMasks(const Evolver& master, float percentVariation);
  void randomise();
  bool getNewValueSimple(int level, int X, int Y);
  bool getNewValue(int level, int X, int Y);
  bool getNewValue2(int level, int X, int Y);
  bool getNewValueParentsOnly(int level, int X, int Y);
  float getNewValueCovolve(int level, int X, int Y);
  void updateCovolveData();
  bool checkAddRemove(int level, int X, int Y, bool addRemove[6][3][3], int i, bool flip);
  void draw();
  void update();
  void reset();
  void set(const Evolver& evolver);
  void read(FILE* fp);
  void write(FILE* fp);

};
