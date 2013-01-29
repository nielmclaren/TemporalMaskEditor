#include "testApp.h"

void testApp::setup() {
	player.loadMovie("looper-closed.mov");
	player.play();
	player.setPaused(true);
	
	previewIndex = 0;
	
	frameCount = player.getTotalNumFrames();
	frameWidth = player.width;
	frameHeight = player.height;
	
	frameToBrushColor = 255 * 255 / (frameCount - 1);
	maxColor = (frameCount - 1) * frameToBrushColor;
	
	maskPixelsDetail = new unsigned short int[frameWidth * frameHeight];
	
	readMask();
	
	cout << "Loading frames..." << endl;
	
	unsigned char* copyPixels;
	for (int i = 0; i < frameCount; i++) {
		cout << i << " of " << frameCount << endl;
		copyPixels = player.getPixels();
		unsigned char* framePixels = new unsigned char[frameWidth * frameHeight * 3];
		for (int i = 0; i < frameWidth * frameHeight * 3; i++) {
			framePixels[i] = copyPixels[i];
		}
		frames.push_back(framePixels);
		player.nextFrame();
	}
	
	brushImage.loadImage("brushes/001-soft.png");
	brushImage.setImageType(OF_IMAGE_GRAYSCALE);
	
	brushColor = maxColor;
	brushFlow = 255;
	brushSize = 50;
	brushStep = 10;
	
	updateBrush();
	
	brushDeltaRemainder = 0;
	
	distorted.setFromPixels(frames[0], frameWidth, frameHeight, OF_IMAGE_COLOR);
	
	isPreviewDragging = false;
	
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
	
	for (int x = 0; x < frameWidth; x++) {
		for (int y = 0; y < frameHeight; y++) {
			int frameIndex = maskPixelsDetail[y * frameWidth + x] / frameToBrushColor;
			unsigned char* frame = frames[frameIndex];
			
			for (int c = 0; c < 3; c++) {
				int pixelIndex = y * frameWidth * 3 + x * 3 + c;
				distortedPixels[pixelIndex] = frame[pixelIndex];
			}
		}
	}
	
	ofSetColor(255, 255, 255);
	
	preview.setFromPixels(frames[previewIndex], frameWidth, frameHeight, OF_IMAGE_COLOR);
	preview.draw(0, 0, frameWidth/2, frameHeight/2);
	
	mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
	mask.draw(frameWidth/2, 0, frameWidth/2, frameHeight/2);
	
	distorted.setFromPixels(distortedPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
	distorted.draw(0, frameHeight/2, frameWidth, frameHeight);
	
	if (mouseY > frameHeight/2) {
		ofEnableAlphaBlending();
		ofNoFill();
		ofSetColor(255, 255, 255, 64);
		ofSetLineWidth(2);
		ofCircle(mouseX, mouseY, brushSize/2);
		ofDisableAlphaBlending();
	}
}

void testApp::exit() {
	for (int i = 0; i < frameCount; i++) {
		delete[] frames[i];
	}
	delete[] maskPixelsDetail;
}

void testApp::readMask() {
	if (mask.loadImage("mask.jpg")) {
		maskPixels = mask.getPixels();
		
		// Make sure mask values are within the expected range.
		for (int i = 0; i < frameWidth * frameHeight; i++) {
			maskPixels[i] = MIN(maskPixels[i], maxColor);
			maskPixelsDetail[i] = maskPixels[i] * 255;
		}
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
	setPreviewIndex(previewIndex + delta);
}

void testApp::setPreviewIndex(int i) {
	previewIndex = i;
	
	if (previewIndex < 0) {
		previewIndex = 0;
	}
	if (previewIndex >= frameCount) {
		previewIndex = frameCount - 1;
	}
	
	brushColor = previewIndex * frameToBrushColor;
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
				int delta = brushColor - maskPixelsDetail[pix];
				if (delta != 0) {
					maskPixelsDetail[pix] += delta / abs(delta)
						* MIN(brushFlow * brushPixels[tPix] / 255.0, abs(delta));
					maskPixels[pix] = maskPixelsDetail[pix] / 255;
				}
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
			brushFlow = MIN(255, brushFlow + 0.5);
			cout << "Brush flow: " << brushFlow << endl;
			break;
			
		case OF_KEY_DOWN:
			brushFlow = MAX(0, brushFlow - 0.5);
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

		case 'z':
			setPreviewIndex(0);
			break;
			
		case 'x':
			setPreviewIndex(frameCount - 1);
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

void testApp::mouseMoved(int x, int y) {
}

void testApp::mouseDragged(int x, int y, int button) {
	if (y < frameHeight/2) {
		if (isPreviewDragging) {
			setPreviewIndex((float) x / (frameWidth/2) * (frameCount - 2));
		}
	}
	else {
		addPoint(x, y - frameHeight/2, true);
	}
}

void testApp::mousePressed(int x, int y, int button) {
	if (y < frameHeight/2 && x < frameWidth/2) {
		setPreviewIndex((float) x / (frameWidth/2) * (frameCount - 2));
		isPreviewDragging = true;
	}
	else {
		addPoint(x, y - frameHeight/2, true);
	}
}

void testApp::mouseReleased(int x, int y, int button) {
	isPreviewDragging = false;
}

void testApp::windowResized(int w, int h) {
}

void testApp::gotMessage(ofMessage msg) {
}

void testApp::dragEvent(ofDragInfo dragInfo) { 
}