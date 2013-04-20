#pragma once

#include "ofMain.h"

// For small speed improvement?
#define ONE_OVER_255 0.00392157

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void clearFrames();
	void readFrames(string);
	void readMask();
	void writeMask();
	void writeDistorted();
	
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
	
	ofVideoPlayer player;
	
	// The base image for the brush.
	ofImage brushImage;
	
	// The resized image used as the actual brush.
	ofImage brush;
	unsigned char* brushPixels;
	
	int brushColor;
	float brushFlow;
	float brushSize;
	float brushStep;
	
	int frameToBrushColor;
	int maxColor;
	
	float prevBrushX;
	float prevBrushY;
	float brushDeltaRemainder;
	
	vector<unsigned char*> frames;
	ofImage frame;
	
	int previewIndex;
	int previewIndexDelta;
	ofImage preview;
	
	unsigned char* maskPixels;
	unsigned short int* maskPixelsDetail;
	ofImage mask;
	
	unsigned char* distortedPixels;
	ofImage distorted;
	
	int frameCount;
	int frameWidth;
	int frameHeight;
	
	bool isPreviewDragging;
	
	unsigned long keyDownTime;
};
