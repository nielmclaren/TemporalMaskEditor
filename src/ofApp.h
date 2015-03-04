#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"

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
  void clearMask();
  void loadMask();
  void saveMask();
  void saveDistorted();

  void updatePreviewIndex(int delta);
  void setPreviewIndex(int i);
  void updateBrush();
  void addPoint(float x, float y, bool newStroke);
  void addBrush(int x, int y);

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  void brushFlowChanged(float & brushFlow);

  // The base image for the brush.
  ofImage brushImage;

  // The resized image used as the actual brush.
  ofImage brush;
  unsigned char* brushPixels;

  int brushColor;

  ofxPanel gui;
  ofxFloatSlider brushFlow;
  ofxFloatSlider brushSize;
  ofxFloatSlider brushStep;

  int frameToBrushColor;
  int maxColor;

  float prevBrushX;
  float prevBrushY;
  float brushDeltaRemainder;

  int previewIndex;
  int previewIndexDelta;

  unsigned char* inputPixels;
  unsigned char* maskPixels;
  unsigned short int* maskPixelsDetail;
  unsigned char* previewPixels;
  unsigned char* outputPixels;

  ofImage preview;
  ofImage mask;
  ofImage distorted;

  int screenWidth;
  int screenHeight;

  int frameWidth;
  int frameHeight;
  int frameCount;

  bool isPreviewDragging;

  unsigned long keyDownTime;
};
