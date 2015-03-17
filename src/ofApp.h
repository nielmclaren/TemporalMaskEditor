#pragma once

#include "ofMain.h"
#include "ofxGui.h"

#include "GradientAnimation.h"
#include "GradientKeyframe.h"
#include "GradientStop.h"

class ofApp : public ofBaseApp {
public:
  void setup();
  void update();
  void draw();
  void exit();

  void updateOutputPixels();
  void saveDistorted();

  int countFrames(string path);
  void clearFrames();
  void loadFrames(string path);
  void clearMask();
  void clearStops();
  void addStop(int, int);

  void clearKeyframes();
  void setKeyframe(int i);

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  void clearGradientButtonClicked();
  void firstKeyframeButtonClicked();
  void lastKeyframeButtonClicked();
  void frameSliderChanged(float& value);
  void intensitySliderChanged(int& value);

  GradientStop* draggingStop;

  GradientAnimation animation;
  int currKeyframeIndex;
  GradientKeyframe* currKeyframe;

  int guiMargin;
  ofxPanel gui;
  ofxGuiGroup globalGui;
  ofxButton clearGradientButton;
  ofxButton firstKeyframeButton;
  ofxButton lastKeyframeButton;
  ofxFloatSlider frameSlider;
  ofxLabel keyframeLabel;
  ofxGuiGroup keyframeGui;

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

