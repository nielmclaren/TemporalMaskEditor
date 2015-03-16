#pragma once

#include "ofMain.h"
#include "GradientKeyframe.h"

class GradientAnimation {
public:
  GradientAnimation();
  ~GradientAnimation();

  void setup(int w, int h);

  void clearKeyframes();
  GradientKeyframe* getKeyframe(int i);
  int numKeyframes();

  void addStop(int x, int y);
  void clearStops();

private:
  int width;
  int height;

  std::vector<GradientKeyframe*> keyframes;
};

