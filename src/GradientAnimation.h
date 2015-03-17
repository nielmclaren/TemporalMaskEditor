#pragma once

#include "ofMain.h"
#include "GradientKeyframe.h"

class GradientAnimation {
public:
  GradientAnimation();
  ~GradientAnimation();

  void setup(int w, int h);
  void draw();
  void render(unsigned short int*, unsigned char*);

  void clearKeyframes();
  GradientKeyframe* getKeyframe(int i);
  int numKeyframes();

  void addStop(int x, int y);
  void clearStops();

  float getTime();
  void setTime(float t);

private:
  GradientStop* getInterpolatedStop(int, float);

  int width;
  int height;
  int numStops;
  float time;

  GradientFrame* currFrame;
  bool currFrameAllocated;

  std::vector<GradientKeyframe*> keyframes;
};

