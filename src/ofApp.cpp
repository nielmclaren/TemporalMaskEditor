#include "ofApp.h"

void ofApp::setup() {
  screenWidth = ofGetWindowWidth();
  screenHeight = ofGetWindowHeight();

  frameWidth = 0;
  frameHeight = 0;
  frameCount = 0;

  inputPixels = NULL;
  maskPixels = NULL;
  maskPixelsDetail = NULL;
  outputPixels = NULL;

  showMask = false;

  draggingStop = NULL;
  currKeyframeIndex = -1;
  currKeyframe = NULL;

  guiMargin = 220;
  gui.setup();

  gui.add(globalGui.setup());
  globalGui.add(clearGradientButton.setup("clear gradient (c)"));
  globalGui.add(firstKeyframeButton.setup("first keyframe"));
  globalGui.add(lastKeyframeButton.setup("last keyframe"));

  clearGradientButton.addListener(this, &ofApp::clearGradientButtonClicked);
  firstKeyframeButton.addListener(this, &ofApp::firstKeyframeButtonClicked);
  lastKeyframeButton.addListener(this, &ofApp::lastKeyframeButtonClicked);

  globalGui.add(keyframeLabel.setup("current keyframe", "0", 200, 20));

  gui.add(keyframeGui.setup());

  loadFrames("shuttle_launch01");
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

    ofPushMatrix();
    ofTranslate(guiMargin, 0);

    ofSetColor(255);
    drawImage.draw(0, 0);

    ofNoFill();
    ofSetColor(0, 255, 0);
    currKeyframe->draw();

    ofPopMatrix();
  }

  gui.draw();
}

void ofApp::exit() {
  clearFrames();
  clearKeyframes();
}

void ofApp::saveDistorted() {
  ofImage distorted;
  distorted.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
  distorted.saveImage("render.jpg", OF_IMAGE_QUALITY_BEST);
}

int ofApp::countFrames(string path) {
  int n = 0;
  ofFile file;
  while (file.doesFileExist(path + "/frame" + ofToString(n + 1, 0, 4, '0') + ".png")) n++;
  return n;
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

void ofApp::clearKeyframes() {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    delete keyframes[i];
  }
}

void ofApp::setKeyframe(int index) {
  int numStops;

  if (currKeyframe) {
    numStops = currKeyframe->numStops();
    for (int i = 0; i < numStops; i++) {
      GradientStop* stop = currKeyframe->getStop(i);
      stop->intensity->removeListener(this, &ofApp::intensitySliderChanged);
    }
  }

  currKeyframeIndex = index;
  currKeyframe = keyframes[index];
  keyframeLabel = ofToString(index);

  keyframeGui.clear();

  numStops = currKeyframe->numStops();
  for (int i = 0; i < numStops; i++) {
    GradientStop* stop = currKeyframe->getStop(i);
    stop->intensity->addListener(this, &ofApp::intensitySliderChanged);
    keyframeGui.add(stop->intensity);
  }
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

  GradientKeyframe* keyframe;

  keyframe = new GradientKeyframe;
  keyframe->setup(frameWidth, frameHeight);
  keyframes.push_back(keyframe);

  keyframe = new GradientKeyframe;
  keyframe->setup(frameWidth, frameHeight);
  keyframes.push_back(keyframe);
  setKeyframe(0);

  frameToBrushColor = 255 * 255 / (frameCount - 1);

  cout << "Loading complete." << endl;
}

void ofApp::clearMask() {
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    maskPixels[i] = maskPixelsDetail[i] = 0;
  }
}

void ofApp::clearStops() {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    GradientKeyframe* keyframe = keyframes[i];
    keyframe->clearStops();
  }

  currKeyframe->updateGradient(maskPixelsDetail, maskPixels);

  keyframeGui.clear();
}

void ofApp::addStop(int x, int y) {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    GradientKeyframe* keyframe = keyframes[i];
    int numStops = keyframe->numStops();
    GradientStop* stop = keyframe->addStop(
        "stop " + ofToString(numStops), x, y,
        numStops % 2 == 0 ? 0 : 65025);

    if (i == currKeyframeIndex) {
      keyframeGui.add(stop->intensity);
      stop->intensity->addListener(this, &ofApp::intensitySliderChanged);
    }
  }
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
      clearStops();
      break;

    case 't':
      showMask = !showMask;
      break;
  }
}

void ofApp::mouseMoved(int x, int y) {
}

void ofApp::mouseDragged(int x, int y, int button) {
  if (draggingStop) {
    draggingStop->pos.set(x - guiMargin, y);
  }
}

void ofApp::mousePressed(int x, int y, int button) {
  draggingStop = NULL;

  draggingStop = currKeyframe->hitTestStops(x - guiMargin, y);
}

void ofApp::mouseReleased(int x, int y, int button) {
  if (draggingStop) {
    draggingStop->pos.set(x - guiMargin, y);
    draggingStop = NULL;
  }
  else {
    addStop(x - guiMargin, y);

    // NOTE: Seems to be a bug where keyframeGui doesn't update.
    keyframeGui.registerMouseEvents();
  }

  currKeyframe->updateGradient(maskPixelsDetail, maskPixels);
}

void ofApp::windowResized(int w, int h) {
}

void ofApp::gotMessage(ofMessage msg) {
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
}

void ofApp::clearGradientButtonClicked() {
  clearStops();
}

void ofApp::firstKeyframeButtonClicked() {
  setKeyframe(0);
}

void ofApp::lastKeyframeButtonClicked() {
  setKeyframe(1);
}

void ofApp::intensitySliderChanged(int& value) {
  currKeyframe->updateGradient(maskPixelsDetail, maskPixels);
}

