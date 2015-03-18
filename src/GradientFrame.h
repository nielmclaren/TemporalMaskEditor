#pragma once

#include "ofMain.h"
#include "GradientStop.h"

#define GRADIENT_STOP_RADIUS 8

class GradientFrame {
public:
  ~GradientFrame();

  virtual void setup(int w, int h);
  virtual void draw();
  virtual void render(unsigned short int* pixelsDetail, unsigned char* pixels);

  GradientStop* addStop(string label, float posx, float posy, int intensity);
  GradientStop* addStop(string label, float posx, float posy, float dirx, float diry, int intensity);
  GradientStop* addStop(GradientStop* stop);
  GradientStop* getStop(int stopIndex);
  int numStops();
  void clearStops();
  void updateStopDirs();

  GradientStop* hitTestStops(int x, int y);

protected:
  void updateGradient(
      unsigned short int* pixelsDetail,
      unsigned char* pixels,
      GradientStop* stop0,
      GradientStop* stop1);

  int findIntersection(float, float, float, float, float, float, float, float, ofVec2f*);
  bool hitTestStop(GradientStop* stop, ofVec2f* test);

  int width;
  int height;

  std::vector<GradientStop*> stops;
};

