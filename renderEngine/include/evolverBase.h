#pragma once
#include "basics.h"

class EvolverBase
{
public:
  void load(char* fileName, int type);
  void draw();
  void update();
  void reset();
  void set(const Evolver& evolver);
  void write(FILE* fp);
};
