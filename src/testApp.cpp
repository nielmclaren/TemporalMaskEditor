#include "testApp.h"

void testApp::setup() {
	player.loadMovie("ddarko-h264.mov");
	player.play();
	player.setPaused(true);
	
	frameCount = player.getTotalNumFrames();
	frameWidth = player.width;
	frameHeight = player.height;
	
	int frameIndex = 0;
	unsigned char* copyPixels;
	while (frameIndex < 30) {
		frameIndex = player.getCurrentFrame();
		cout << frameIndex << endl;
		player.nextFrame();
		
		copyPixels = player.getPixels();
		unsigned char* framePixels = (unsigned char*) malloc(frameWidth * frameHeight * 3);
		for (int i = 0; i < frameWidth * frameHeight * 3; i++) {
			framePixels[i] = copyPixels[i];
		}
		frames.push_back(framePixels);
	}
	
	readMask();
	
	distorted.setFromPixels(frames[0], frameWidth, frameHeight, OF_IMAGE_COLOR);
	
	cout << "setup complete." << endl;
}

void testApp::update() {
}

void testApp::draw() {
	distortedPixels = distorted.getPixels();
	
	int numFrames = frames.size();
	for (int x = 0; x < frameWidth; x++) {
		for (int y = 0; y < frameHeight; y++) {
			for (int c = 0; c < 3; c++) {
				float maskFraction = maskPixels[y * frameWidth + x] / 255.0;
				int frameIndex = maskFraction * (numFrames - 1);
				//float remainder = (maskFraction * (numFrames - 1) - frameIndex) / (numFrames - 1);
				
				int pixelIndex = y * frameWidth * 3 + x * 3 + c;
				unsigned char* frame = frames[frameIndex];
				//unsigned char* nextFrame = frames[frameIndex + 1];
				distortedPixels[pixelIndex] = frame[pixelIndex];
			}
		}
	}
	
	mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
	mask.draw(10, 10, 640, 272);
	
	distorted.setFromPixels(distortedPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
	distorted.draw(10, 292, 1280, 544);
}

void testApp::exit() {
	for (int i = 0; i < frames.size(); i++) {
		delete[] frames[i];
	}
}

void testApp::readMask() {
	mask.loadImage("mask.jpg");
	maskPixels = mask.getPixels();
}

void testApp::writeMask() {
	mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
	mask.saveImage("mask.jpg");
}

void testApp::keyPressed(int key) {
	switch (key) {
		case 'r':
			readMask();
			break;
			
		case 'w':
			writeMask();
			break;
	}
}

void testApp::keyReleased(int key) {
}

void testApp::mouseMoved(int x, int y) {
}

void testApp::mouseDragged(int x, int y, int button) {
}

void testApp::mousePressed(int x, int y, int button) {
}

void testApp::mouseReleased(int x, int y, int button) {
}

void testApp::windowResized(int w, int h) {
}

void testApp::gotMessage(ofMessage msg) {
}

void testApp::dragEvent(ofDragInfo dragInfo) { 
}