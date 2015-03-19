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
  globalGui.add(frameSlider.setup("frame", 0, 0, 1));

  clearGradientButton.addListener(this, &ofApp::clearGradientButtonClicked);
  firstKeyframeButton.addListener(this, &ofApp::firstKeyframeButtonClicked);
  lastKeyframeButton.addListener(this, &ofApp::lastKeyframeButtonClicked);
  frameSlider.addListener(this, &ofApp::frameSliderChanged);

  globalGui.add(keyframeLabel.setup("current keyframe", "0", 200, 20));

  gui.add(keyframeGui.setup());

  folderNamer.setup("output/output", "/");

  loadFrames("shuttle_launch01");
}

void ofApp::update() {}

void ofApp::draw() {
  ofBackground(0);

  if (frameWidth > 0) {
    if (showMask) {
      drawImage.setFromPixels(maskPixels, frameWidth, frameHeight, OF_IMAGE_GRAYSCALE);
    }
    else {
      updateOutputPixels();
      drawImage.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
    }

    ofPushMatrix();
    ofTranslate(guiMargin, 0);

    ofSetColor(255);
    drawImage.draw(0, 0);

    ofNoFill();
    ofSetColor(0, 255, 0);
    animation.draw();

    ofPopMatrix();
  }

  gui.draw();
}

void ofApp::exit() {
  clearFrames();
  animation.clearKeyframes();
}

void ofApp::updateOutputPixels() {
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    int frameIndex = maskPixelsDetail[i] / frameToBrushColor;
    for (int c = 0; c < 3; c++) {
      outputPixels[i * 3 + c] = inputPixels[frameIndex * frameWidth * frameHeight * 3 + i * 3 + c];
    }
  }
}

void ofApp::saveDistorted() {
  cout << "Writing frame... ";

  ofImage distorted;
  distorted.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
  distorted.saveImage("render.tif", OF_IMAGE_QUALITY_BEST);

  cout << "done." << endl;
}

void ofApp::saveDistortedAnimation() {
  cout << "Writing animation... ";

  string folderName = folderNamer.next();
  ofImage distorted;
  int numFrames = 25;
  for (int i = 0; i < numFrames; i++) {
    float t = (float)i / numFrames;
    animation.setTime(t);
    animation.render(maskPixelsDetail, maskPixels);
    updateOutputPixels();
    distorted.setFromPixels(outputPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
    distorted.saveImage(folderName + "/render" + ofToString(i, 4, '0') + ".gif", OF_IMAGE_QUALITY_BEST);
  }

  cout << "done." << endl;
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

void ofApp::setKeyframe(int index) {
  int numStops;

  if (currKeyframe) {
    numStops = currKeyframe->numStops();
    for (int i = 0; i < numStops; i++) {
      GradientStop* stop = currKeyframe->getStop(i);
      stop->intensity->removeListener(this, &ofApp::intensitySliderChanged);
    }
  }
  keyframeGui.clear();

  currKeyframeIndex = index;
  if (index < 0) {
    currKeyframe = NULL;
    keyframeLabel = "none";
  }
  else {
    currKeyframe = animation.getKeyframe(index);
    keyframeLabel = ofToString(index);

    numStops = currKeyframe->numStops();
    for (int i = 0; i < numStops; i++) {
      GradientStop* stop = currKeyframe->getStop(i);
      stop->intensity->addListener(this, &ofApp::intensitySliderChanged);
      keyframeGui.add(stop->intensity);
    }
  }
}

void ofApp::loadFrames(string path) {
  ofImage image;

  clearFrames();

  image.loadImage(path + "/frame0001.png");
  frameWidth = image.width;
  frameHeight = image.height;
  frameCount = countFrames(path);

  animation.setup(frameWidth, frameHeight);

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

  frameSlider = 0;

  frameToBrushColor = 255 * 255 / (frameCount - 1);

  cout << "Loading complete." << endl;
}

void ofApp::clearMask() {
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    maskPixels[i] = maskPixelsDetail[i] = 0;
  }
}

void ofApp::clearStops() {
  animation.clearStops();
  if (currKeyframe) {
    currKeyframe->render(maskPixelsDetail, maskPixels);
  }

  keyframeGui.clear();
}

void ofApp::addStop(int x, int y) {
  animation.addStop(x, y);

  if (currKeyframe) {
    GradientStop* stop = currKeyframe->getStop(currKeyframe->numStops() - 1);
    keyframeGui.add(stop->intensity);
    stop->intensity->addListener(this, &ofApp::intensitySliderChanged);
  }
}

void ofApp::incrementFrame() {
  int numFrames = 10;
  int i = floor(frameSlider * numFrames) + 1;
  if (i >= numFrames) i = 0;
  frameSlider = (float)i / numFrames;
}

void ofApp::keyPressed(int key) {
}

void ofApp::keyReleased(int key) {
  switch (key) {
    case 'r':
      saveDistorted();
      break;

    case 'e':
      saveDistortedAnimation();
      break;

    case 'c':
      clearStops();
      break;

    case 't':
      showMask = !showMask;
      break;

    case ' ':
      incrementFrame();
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

  if (currKeyframe) {
    draggingStop = currKeyframe->hitTestStops(x - guiMargin, y);
  }
}

void ofApp::mouseReleased(int x, int y, int button) {
  if (currKeyframe) {
    if (draggingStop) {
      draggingStop->pos.set(x - guiMargin, y);
      draggingStop = NULL;
      currKeyframe->updateStopDirs();
    }
    else {
      addStop(x - guiMargin, y);

      // NOTE: Seems to be a bug where keyframeGui doesn't update.
      keyframeGui.registerMouseEvents();
    }

    currKeyframe->render(maskPixelsDetail, maskPixels);
  }
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
  frameSlider = 0;
}

void ofApp::lastKeyframeButtonClicked() {
  frameSlider = 1;
}

void ofApp::frameSliderChanged(float& value) {
  if (value == 0) {
    setKeyframe(0);
  }
  else if (value == 1) {
    setKeyframe(1);
  }
  else {
    setKeyframe(-1);
  }

  animation.setTime(value);
  animation.render(maskPixelsDetail, maskPixels);
}

void ofApp::intensitySliderChanged(int& value) {
  if (currKeyframe) {
    currKeyframe->render(maskPixelsDetail, maskPixels);
  }
}

