#include "ofApp.h"

void ofApp::setup() {
  screenWidth = ofGetWindowWidth();
  screenHeight = ofGetWindowHeight();

  frameWidth = 0;
  frameHeight = 0;
  frameCount = 0;

  loadSettings();

  inputPixels = NULL;
  maskPixels = NULL;
  maskPixelsDetail = NULL;
  outputPixels = NULL;

  gui.setup();
  gui.add(gradientStartIntensity.setup("gradient start", 0, 0, 65025));
  gui.add(gradientEndIntensity.setup("gradient end", 65025, 0, 65025));

  gradientStartIntensity.addListener(this, &ofApp::gradientIntensityChanged);
  gradientEndIntensity.addListener(this, &ofApp::gradientIntensityChanged);

  loadFrames("adam_magyar_stainless01");

  gradientStartX = 100;
  gradientStartY = 200;
  gradientEndX = 500;
  gradientEndY = 400;
  drawGradient();
}

void ofApp::update() {
}

void ofApp::draw() {
  ofBackground(0);

  if (frameWidth > 0) {
    for (int i = 0; i < frameWidth * frameHeight; i++) {
      int frameIndex = maskPixelsDetail[i] / frameToBrushColor;
      for (int c = 0; c < 3; c++) {
        outputPixels[i * 3 + c] = inputPixels[frameIndex * frameWidth * frameHeight * 3 + i * 3 + c];
      }
    }

    ofSetColor(255);

    mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
    mask.draw(frameWidth/2, 0, frameWidth/2, frameHeight/2);

    distorted.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
    distorted.draw(0, frameHeight/2, frameWidth, frameHeight);

    ofFill();
    ofSetColor(255, 0, 0, 64);
    ofEnableAlphaBlending();
    ofCircle(gradientStartX, frameHeight/2 + gradientStartY, 10);
    ofCircle(gradientEndX, frameHeight/2 + gradientEndY, 10);
    ofDisableAlphaBlending();
  }

  gui.draw();
}

void ofApp::exit() {
  clearFrames();
}

void ofApp::loadSettings() {
  ofxXmlSettings settings;
  settings.loadFile("settings.xml");
}

void ofApp::saveSettings() {
  ofxXmlSettings settings;
  settings.saveFile("settings.xml");
}

void ofApp::clearFrames() {
  if (inputPixels != NULL) delete[] inputPixels;
  if (maskPixels != NULL) delete[] maskPixels;
  if (maskPixelsDetail != NULL) delete[] maskPixelsDetail;
  if (outputPixels != NULL) delete[] outputPixels;

  inputPixels = NULL;
  maskPixels = NULL;
  maskPixelsDetail = NULL;
  outputPixels = NULL;

  frameCount = 0;
  frameWidth = 0;
  frameHeight = 0;

  frameToBrushColor = 0;
}

void ofApp::loadFrames(string path) {
  ofImage image;

  clearFrames();

  image.loadImage(path + "/frame0001.png");
  frameWidth = image.width;
  frameHeight = image.height;
  frameCount = countFrames(path);

  cout << "Loading " << frameCount << " frames " << endl
    << "\tPath: " << path << endl
    << "\tDimensions: " << frameWidth << "x" << frameHeight << endl
    << "\tSize: " << floor(frameCount * frameWidth * frameHeight * 3 / 1024 / 1024) << " MB" << endl;

  inputPixels = new unsigned char[frameCount * frameWidth * frameHeight * 3];
  for (int frameIndex = 0; frameIndex < frameCount; frameIndex++) {
    image.loadImage(path + "/frame" + ofToString(frameIndex + 1, 0, 4, '0') + ".png");
    image.setImageType(OF_IMAGE_COLOR);

    for (int i = 0; i < frameWidth * frameHeight * 3; i++) {
      inputPixels[frameIndex * frameWidth * frameHeight * 3 + i] = image.getPixels()[i];
    }
  }

  maskPixels = new unsigned char[frameWidth * frameHeight * 1];
  maskPixelsDetail = new unsigned short int[frameWidth * frameHeight * 1];
  outputPixels = new unsigned char[frameWidth * frameHeight * 3];

  clearMask();

  frameToBrushColor = 255 * 255 / (frameCount - 1);

  cout << "Loading complete." << endl;
}

void ofApp::clearMask() {
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    maskPixels[i] = maskPixelsDetail[i] = 0;
  }
}

void ofApp::loadMask() {
  if (mask.loadImage("mask.png")) {
    float maxColor = (frameCount - 1) * frameToBrushColor;
    unsigned char* loadMaskPixels = mask.getPixels();
    for (int i = 0; i < frameWidth * frameHeight; i++) {
      maskPixels[i] = MIN(loadMaskPixels[i], maxColor);
      maskPixelsDetail[i] = maskPixels[i] * 255;
    }
  }
  else {
    cout << "Warning: problem loading mask." << endl;
  }
}

void ofApp::saveMask() {
  mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
  mask.saveImage("mask.png");
}

void ofApp::saveDistorted() {
  distorted.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
  distorted.saveImage("render.jpg", OF_IMAGE_QUALITY_BEST);
}

void ofApp::drawGradient() {
  ofVec2f start, end, u, v, w, curr;
  start.set(gradientStartX, gradientStartY);
  end.set(gradientEndX, gradientEndY);
  v = end - start;
  float d = v.length();

  for (int x = 0; x < frameWidth; x++) {
    for (int y = 0; y < frameHeight; y++) {
      int i = y * frameWidth + x;
      curr.set(x, y);
      u = curr - start;
      w = u.dot(v) * v / v.length() / v.length();

      if (u.dot(v) < 0) {
        maskPixelsDetail[i] = gradientStartIntensity;
      }
      else {
        maskPixelsDetail[i] = ofMap(
          w.length() / d,
          0, 1,
          gradientStartIntensity, gradientEndIntensity,
          true);
      }

      maskPixels[i] = maskPixelsDetail[i] / 255;
    }
  }
}

int ofApp::countFrames(string path) {
  int n = 0;
  ofFile file;
  while (file.doesFileExist(path + "/frame" + ofToString(n + 1, 0, 4, '0') + ".png")) n++;
  return n;
}

void ofApp::keyPressed(int key) {
}

void ofApp::keyReleased(int key) {
  switch (key) {
    case 'r':
      loadMask();
      break;

    case 'w':
      saveMask();
      break;

    case 's':
      saveDistorted();
      break;

    case 'x':
      clearFrames();
      break;
  }
}

void ofApp::mouseMoved(int x, int y) {
}

void ofApp::mouseDragged(int x, int y, int button) {
}

void ofApp::mousePressed(int x, int y, int button) {
  if (y > frameHeight/2) {
    gradientStartX = x;
    gradientStartY = y - frameHeight/2;
  }
}

void ofApp::mouseReleased(int x, int y, int button) {
  gradientEndX = x;
  gradientEndY = y - frameHeight/2;
  drawGradient();
}

void ofApp::windowResized(int w, int h) {
}

void ofApp::gotMessage(ofMessage msg) {
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
}

void ofApp::gradientIntensityChanged(float & value){
  drawGradient();
}

