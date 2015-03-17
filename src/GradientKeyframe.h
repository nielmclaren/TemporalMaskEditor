#pragma once

#include "ofMain.h"

#include "GradientFrame.h"
#include "GradientStop.h"

#define GRADIENT_STOP_RADIUS 8

class GradientKeyframe : public GradientFrame {
public:
  GradientKeyframe();
  ~GradientKeyframe();
  void draw();

private:
};

