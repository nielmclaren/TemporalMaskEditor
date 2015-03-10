#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"
#include "GradientStop.h"

class ofApp : public ofBaseApp {
public:
  void setup();
  void update();
  void draw();
  void exit();

  void saveFrame();
  void loadSettings();
  void saveSettings();
  int countFrames(string path);
  void clearFrames();
  void loadFrames(string path);
  void clearGradient();
  void clearMask();
  void loadMask();
  void saveMask();
  void saveDistorted();

  void updateGradient();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  void clearGradientClicked();
  void gradientIntensityChanged(float &value);

  std::vector<GradientStop> gradientStops;

  int guiMargin;
  ofxPanel gui;
  ofxButton clearGradientButton;

  int frameToBrushColor;

  unsigned char* inputPixels;
  unsigned char* maskPixels;
  unsigned short int* maskPixelsDetail;
  unsigned char* outputPixels;

  bool showMask;
  ofImage drawImage;

  int screenWidth;
  int screenHeight;

  int frameWidth;
  int frameHeight;
  int frameCount;
};
