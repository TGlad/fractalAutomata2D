#pragma once
#include "basics.h"
#include "View.h"
#include "Screen.h"

class Core
{
public:
  void init(int screenWidth, int screenHeight);
  void deinit();
  void render();
  void update(TimePeriod timeStep);
  Screen* screen;
  View* view;
};
