#pragma once

#include "ofMain.h"

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
	void exit();
	
	void readMask();
	void writeMask();
	
	void setupBrushes(int w, int h);
	void updateBrushSettings(bool first);
	void managePainting();
	
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
	
	vector<unsigned char*> frames;
	ofImage frame;
	
	ofImage brushed;
	
	unsigned char* maskPixels;
	ofImage mask;
	
	unsigned char* distortedPixels;
	ofImage distorted;
	
	ofTexture texture;
	
	int frameCount;
	int frameWidth;
	int frameHeight;
};
