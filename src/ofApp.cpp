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

  showMask = false;

  guiMargin = 220;
  gui.setup();
  clearGradientButton.setup("clear gradient (c)");
  clearGradientButton.addListener(this, &ofApp::clearGradientClicked);
  clearGradient();

  loadFrames("adam_magyar_stainless01");
}

void ofApp::update() {
}

void ofApp::draw() {
  ofBackground(0);

  if (frameWidth > 0) {
    if (showMask) {
      drawImage.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
    }
    else {
      for (int i = 0; i < frameWidth * frameHeight; i++) {
        int frameIndex = maskPixelsDetail[i] / frameToBrushColor;
        for (int c = 0; c < 3; c++) {
          outputPixels[i * 3 + c] = inputPixels[frameIndex * frameWidth * frameHeight * 3 + i * 3 + c];
        }
      }

      drawImage.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
    }

    ofSetColor(255);
    drawImage.draw(guiMargin, 0);

    ofFill();
    ofSetColor(255, 0, 0, 128);
    ofEnableAlphaBlending();
    for (int i = 0; i < gradientStops.size(); i++) {
      GradientStop stop = gradientStops[i];
      ofCircle(guiMargin + stop.pos.x, stop.pos.y, 10);
    }
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

void ofApp::clearGradient() {
  for (int i = 0; i < gradientStops.size(); i++) {
    GradientStop stop = gradientStops[i];
    stop.intensity->removeListener(this, &ofApp::gradientIntensityChanged);
  }

  gradientStops = std::vector<GradientStop>();
  gui.add(&clearGradientButton);

  clearMask();
}

void ofApp::clearMask() {
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    maskPixels[i] = maskPixelsDetail[i] = 0;
  }
}

void ofApp::loadMask() {
  ofImage mask;
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
  ofImage mask;
  mask.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
  mask.saveImage("mask.png");
}

void ofApp::saveDistorted() {
  ofImage distorted;
  distorted.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
  distorted.saveImage("render.jpg", OF_IMAGE_QUALITY_BEST);
}

void ofApp::updateGradient() {
  if (gradientStops.size() < 2) return;

  float gradientStartIntensity = *(gradientStops[0].intensity);
  float gradientEndIntensity = *(gradientStops[1].intensity);

  ofVec2f start, end, u, v, w, curr;
  start = gradientStops[0].pos;
  end = gradientStops[1].pos;
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
    case ' ':
      saveDistorted();
      break;

    case 'c':
      clearGradient();
      break;

    case 'l':
      loadMask();
      break;

    case 's':
      saveMask();
      break;

    case 't':
      showMask = !showMask;
      break;
  }
}

void ofApp::mouseMoved(int x, int y) {
}

void ofApp::mouseDragged(int x, int y, int button) {
}

void ofApp::mousePressed(int x, int y, int button) {
}

void ofApp::mouseReleased(int x, int y, int button) {
  int i = gradientStops.size();
  GradientStop stop;
  stop.pos.set(x - guiMargin, y);
  stop.intensity = new ofxFloatSlider;
  stop.intensity->setup("stop " + ofToString(i) + " intensity", i % 2 == 0 ? 0 : 65025, 0, 65025);

  gui.add(stop.intensity);
  stop.intensity->addListener(this, &ofApp::gradientIntensityChanged);

  gradientStops.push_back(stop);
  updateGradient();
}

void ofApp::windowResized(int w, int h) {
}

void ofApp::gotMessage(ofMessage msg) {
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
}

void ofApp::clearGradientClicked() {
  clearGradient();
}

void ofApp::gradientIntensityChanged(float & value){
  updateGradient();
}

