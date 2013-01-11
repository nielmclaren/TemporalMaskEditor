#include "testApp.h"

void testApp::setup() {
	player.loadMovie("ddarko-jetengine.mov");
	player.play();
	player.setPaused(true);
	
	previewIndex = 0;
	
	readMask();
	
	frameCount = player.getTotalNumFrames();
	frameWidth = player.width;
	frameHeight = player.height;
	
	cout << "Loading frames..." << endl;
	
	int frameIndex = 0;
	unsigned char* copyPixels;
	for (int i = 0; i < frameCount; i++) {
		cout << i << " of " << frameCount << endl;
		copyPixels = player.getPixels();
		unsigned char* framePixels = (unsigned char*) malloc(frameWidth * frameHeight * 3);
		for (int i = 0; i < frameWidth * frameHeight * 3; i++) {
			framePixels[i] = copyPixels[i];
		}
		frames.push_back(framePixels);
		player.nextFrame();
	}
	
	brushImage.loadImage("brushes/001-soft.png");
	brushImage.setImageType(OF_IMAGE_GRAYSCALE);
	
	brushColor = 0xff;
	brushFlow = 0.04;
	brushSize = 50;
	brushStep = 10;
	
	updateBrush();
	
	brushDeltaRemainder = 0;
	
	distorted.setFromPixels(frames[0], frameWidth, frameHeight, OF_IMAGE_COLOR);
	
	cout << "setup complete." << endl;
}

void testApp::update() {
	// Delay before playback
	if (ofGetSystemTime() - keyDownTime > 400) {
		updatePreviewIndex(previewIndexDelta);
	}
}

void testApp::draw() {
	distortedPixels = distorted.getPixels();
	
	int range = frameCount - 2;
	for (int x = 0; x < frameWidth; x++) {
		for (int y = 0; y < frameHeight; y++) {
			for (int c = 0; c < 3; c++) {
				float maskFraction = maskPixels[y * frameWidth + x] / 255.0;
				int frameIndex = maskFraction * range;
				float remainder = (maskFraction * range - frameIndex) / range;
				
				int pixelIndex = y * frameWidth * 3 + x * 3 + c;
				unsigned char* frame = frames[frameIndex];
				unsigned char* nextFrame = frames[frameIndex + 1];
				distortedPixels[pixelIndex] = frame[pixelIndex]; // * (remainder) + nextFrame[pixelIndex] * (1 - remainder);
			}
		}
	}
	
	preview.setFromPixels(frames[previewIndex], frameWidth, frameHeight, OF_IMAGE_COLOR);
	preview.draw(0, 0, 960, 404);
	
	mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
	mask.draw(960, 0, 960, 404);
	
	distorted.setFromPixels(distortedPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
	distorted.draw(0, 404, 1920, 808);
	
	int shade = previewIndex / (float) frameCount * 255;
	ofColor c(shade, shade, shade);
	int hexColor = c.getHex();
	stringstream ss;
	ss << hex << hexColor;
	
	ofSetColor(255, 255, 255);
	ofDrawBitmapString(ss.str(), 10, 20);
}

void testApp::exit() {
	for (int i = 0; i < frameCount; i++) {
		delete[] frames[i];
	}
}

void testApp::readMask() {
	if (mask.loadImage("mask.jpg")) {
		maskPixels = mask.getPixels();
	}
	else {
		cout << "Warning: problem loading mask." << endl;
	}
}

void testApp::writeMask() {
	mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
	mask.saveImage("mask.jpg");
}

void testApp::writeDistorted() {
	distorted.setFromPixels(distortedPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
	distorted.saveImage("distorted.tga", OF_IMAGE_QUALITY_BEST);
}

void testApp::updatePreviewIndex(int delta) {
	previewIndexDelta = delta;
	previewIndex += delta;
	
	if (previewIndex < 0) {
		previewIndex = 0;
	}
	if (previewIndex >= frameCount) {
		previewIndex = frameCount - 1;
	}
}

void testApp::updateBrush() {
	brush.clone(brushImage);
	brush.setImageType(OF_IMAGE_GRAYSCALE);
	brush.resize(brushSize, brushSize);
	brushPixels = brush.getPixels();
}

void testApp::addPoint(float startX, float startY, bool newStroke) {
	if (newStroke) {
		// Only draw one brush stroke.
		addBrush(startX - brush.width/2, startY - brush.width/2);
		
		brushDeltaRemainder = brushStep;
	}
	else {
		float deltaX = (float) (startX - prevBrushX);
		float deltaY = (float) (startY - prevBrushY);
		float delta = sqrt(deltaX * deltaX + deltaY * deltaY);
		
		int d;
		for (d = brushDeltaRemainder; d < delta; d += brushStep) {
			addBrush(
					 prevBrushX + d * deltaX / delta - brush.width/2,
					 prevBrushY + d * deltaY / delta - brush.height/2);
		}
		
		brushDeltaRemainder = d - delta;
	}
	
	prevBrushX = startX;
	prevBrushY = startY;
}

void testApp::addBrush(int tx, int ty) {
	int pix = 0;
	
	// This is what we use to move through the brushNumber array
	int tPix = 0;
	
	int destX = brush.width  + tx;
	int destY = brush.height + ty;
	
	// Lets check that we don't draw out outside the projected image
	if (destX >= frameWidth) destX = frameWidth - 1;
	if (destY >= frameHeight) destY = frameHeight - 1;
	
	// If the brushNumber is a bit off the screen on the left side
	// we need to figure this amount out so we only copy part
	// of the brushNumber image
	int offSetCorrectionLeft = 0;
	if (tx < 0) {
		offSetCorrectionLeft = -tx;
		tx = 0;
	}
	
	// Same here for y - we need to figure out the y offset
	// for the cropped brush
	if (ty < 0) {
		tPix = -ty * brush.width;
		ty = 0;
	}
	
	// This is for the right hand side cropped brush
	int offSetCorrectionRight = brush.width + tx - destX;
	tPix += offSetCorrectionLeft;
	
	for (int y = ty; y < destY; y++) {
		for (int x = tx; x < destX; x++) {
			pix = x + y * frameWidth;
			if (brushPixels[tPix] != 0) {
				float value = brushPixels[tPix] / 255.0 * brushFlow;
				maskPixels[pix] = maskPixels[pix] * (1 - value) + brushColor * value;
			}
			tPix++;
		}
		tPix += offSetCorrectionRight;
	}
}

void testApp::keyPressed(int key) {
	keyDownTime = ofGetSystemTime();
	switch (key) {
		case OF_KEY_RIGHT:
			updatePreviewIndex(1);
			break;
			
		case OF_KEY_LEFT:
			updatePreviewIndex(-1);
			break;
	}
}

void testApp::keyReleased(int key) {
	switch (key) {
		case OF_KEY_RIGHT:
			updatePreviewIndex(0);
			break;
		
		case OF_KEY_LEFT:
			updatePreviewIndex(0);
			break;
			
		case OF_KEY_UP:
			brushFlow = MIN(1, brushFlow + 0.005);
			cout << "Brush flow: " << brushFlow << endl;
			break;
			
		case OF_KEY_DOWN:
			brushFlow = MAX(0, brushFlow - 0.005);
			cout << "Brush flow: " << brushFlow << endl;
			break;
			
		case 'r':
			readMask();
			break;
			
		case 'w':
			writeMask();
			break;
			
		case 's':
			writeDistorted();
			break;

		case 'x':
			brushColor = brushColor == 0xff ? 0 : 0xff;
			break;
			
		case 'v':
			brushSize += 5;
			cout << "Brush size: " << brushSize << endl;
			updateBrush();
			break;
		
		case 'c':
			brushSize -= 5;
			cout << "Brush size: " << brushSize << endl;
			updateBrush();
			break;
	}
}

int testApp::screenToFrameX(int x, int y) {
	if (y < 404) {
		if (x < 960) return (float) x / 960 * 1920;
		return (float) (x - 960) / 960 * 1920;
	}
	else {
		return x;
	}
}

int testApp::screenToFrameY(int x, int y) {
	if (y < 404) return (float) y / 404 * 808;
	return y - 404;
}

void testApp::mouseMoved(int x, int y) {
}

void testApp::mouseDragged(int x, int y, int button) {
	x = screenToFrameX(x, y);
	y = screenToFrameY(x, y);
	addPoint(x, y, false);
}

void testApp::mousePressed(int x, int y, int button) {
	x = screenToFrameX(x, y);
	y = screenToFrameY(x, y);
	addPoint(x, y, true);
}

void testApp::mouseReleased(int x, int y, int button) {
}

void testApp::windowResized(int w, int h) {
}

void testApp::gotMessage(ofMessage msg) {
}

void testApp::dragEvent(ofDragInfo dragInfo) { 
}