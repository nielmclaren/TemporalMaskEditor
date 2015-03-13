#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "GradientStop.h"

#define GRADIENT_STOP_RADIUS 8

class GradientKeyframe {
public:
  GradientKeyframe();
  ~GradientKeyframe();

  void setup(int w, int h);
  void draw();

  void setGuiGroup(ofxGuiGroup*);

  GradientStop* addStop(string label, float posx, float posy, int intensity);
  GradientStop* addStop(string label, float posx, float posy, float dirx, float diry, int intensity);
  void updateStopPos(int stopIndex, float x, float y);
  void updateStopDir(int stopIndex, float x, float y);
  void updateStopIntensity(int stopIndex, int intensity);
  int numStops();
  void clearStops();

  void updateGradient(unsigned short int* pixelsDetail, unsigned char* pixels);

  GradientStop* hitTestStops(int x, int y);

private:
  int width;
  int height;

  std::vector<GradientStop*> stops;

  ofxGuiGroup* gui;

  void updateGradient(
      unsigned short int* pixelsDetail,
      unsigned char* pixels,
      GradientStop* stop0,
      GradientStop* stop1);

  int findIntersection(float, float, float, float, float, float, float, float, ofVec2f*);
  bool hitTestStop(GradientStop* stop, ofVec2f* test);
};

