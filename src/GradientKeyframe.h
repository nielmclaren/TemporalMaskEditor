#pragma once

#include "ofMain.h"
#include "GradientStop.h"

#define GRADIENT_STOP_RADIUS 8

class GradientKeyframe {
public:
  GradientKeyframe();
  ~GradientKeyframe();

  void setup(int w, int h);
  void draw();

  GradientStop* addStop(string label, float posx, float posy, int intensity);
  GradientStop* addStop(string label, float posx, float posy, float dirx, float diry, int intensity);
  GradientStop* getStop(int stopIndex);
  int numStops();
  void clearStops();

  void updateStopDirs();
  void updateGradient(unsigned short int* pixelsDetail, unsigned char* pixels);

  GradientStop* hitTestStops(int x, int y);

private:
  int width;
  int height;

  std::vector<GradientStop*> stops;

  void updateGradient(
      unsigned short int* pixelsDetail,
      unsigned char* pixels,
      GradientStop* stop0,
      GradientStop* stop1);

  int findIntersection(float, float, float, float, float, float, float, float, ofVec2f*);
  bool hitTestStop(GradientStop* stop, ofVec2f* test);
};

