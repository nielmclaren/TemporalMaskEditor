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

  draggingStopIndex = -1;

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

    ofNoFill();
    ofSetColor(128);
    for (int i = 0; i < gradientStops.size(); i++) {
      GradientStop stop = gradientStops[i];
      ofVec2f d = stop.dir.getScaled(15);
      ofCircle(guiMargin + stop.pos.x, stop.pos.y, GRADIENT_STOP_RADIUS);
      ofLine(guiMargin + stop.pos.x, stop.pos.y, guiMargin + stop.pos.x + d.x, stop.pos.y + d.y);
    }
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
  gui.clear();
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
  int numStops = gradientStops.size();
  ofVec2f d;

  for (int i = 0; i < frameWidth * frameHeight; i++) {
    maskPixelsDetail[i] = maskPixels[i] = 0;
  }

  for (int i = 0; i < numStops; i++) {
    gradientStops[i].dir.zero();
  }

  if (numStops < 2) return;

  for (int i = 0; i < numStops; i++) {
    GradientStop* curr = &gradientStops[i];

    if (i > 0) {
      GradientStop prev = gradientStops[i - 1];
      curr->dir += curr->pos - prev.pos;
    }
    if (i < numStops - 1) {
      GradientStop next = gradientStops[i + 1];
      curr->dir += next.pos - curr->pos;
    }

    curr->dir.normalize();
  }

  for (int i = 0; i < numStops - 1; i++) {
    GradientStop stop0 = gradientStops[i];
    GradientStop stop1 = gradientStops[i + 1];

    updateGradient(stop0, stop1);
  }
}

// TODO: Pass by ref prolly, aye?
void ofApp::updateGradient(GradientStop stop0, GradientStop stop1) {
  ofVec2f p0 = stop0.pos;
  ofVec2f p1 = stop1.pos;
  ofVec2f perp0(-stop0.dir.y, stop0.dir.x);
  ofVec2f perp1(-stop1.dir.y, stop1.dir.x);
  ofVec2f curr;
  ofVec2f u, w, v = p1 - p0;
  float d = v.length();
  float b;
  float value;
  ofVec2f intersect0, intersect1;
  float intersectDist, intersectDist0, intersectDist1;
  int i;

  for (int x = 0; x < frameWidth; x++) {
    for (int y = 0; y < frameHeight; y++) {
      i = y * frameWidth + x;
      curr.set(x, y);
      u = curr - p0;
      u.set(-u.y, u.x);
      w = v.getScaled(u.dot(v) / v.length() / v.length());

      if (findIntersection(
          x, y, x + v.x, y + v.y,
          p0.x, p0.y, p0.x + perp0.x, p0.y + perp0.y, &intersect0)
        || findIntersection(
          x, y, x + v.x, y + v.y,
          p1.x, p1.y, p1.x + perp1.x, p1.y + perp1.y, &intersect1)) {
        // FIXME: Handle parallel lines.
        cout << "Lines were parallel." << endl;
      }

      intersectDist = (intersect1 - intersect0).length();
      intersectDist0 = (curr - intersect0).length();
      intersectDist1 = (curr - intersect1).length();

      if (intersectDist0 < intersectDist && intersectDist1 < intersectDist) {
        value = ofMap(
          intersectDist0 / intersectDist,
          0, 1,
          *(stop0.intensity), *(stop1.intensity),
          true);
        maskPixelsDetail[i] = MAX(maskPixelsDetail[i], value);
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
  if (draggingStopIndex >= 0) {
    gradientStops[draggingStopIndex].pos.set(x - guiMargin, y);
  }
}

void ofApp::mousePressed(int x, int y, int button) {
  ofVec2f mouse(x - guiMargin, y);

  draggingStopIndex = -1;

  int numStops = gradientStops.size();
  for (int i = 0; i < numStops; i++) {
    GradientStop stop = gradientStops[i];
    if (hitTestGradientStop(&stop, &mouse)) {
      draggingStopIndex = i;
    }
  }
}

void ofApp::mouseReleased(int x, int y, int button) {
  if (draggingStopIndex >= 0) {
    gradientStops[draggingStopIndex].pos.set(x - guiMargin, y);
    draggingStopIndex = -1;
    updateGradient();
  }
  else {
    int numStops = gradientStops.size();
    GradientStop stop;
    stop.pos.set(x - guiMargin, y);
    stop.intensity = new ofxFloatSlider;
    stop.intensity->setup(
        "stop " + ofToString(numStops),
        numStops % 2 == 0 ? 0 : 65025, 0, 65025);

    gui.add(stop.intensity);
    stop.intensity->addListener(this, &ofApp::gradientIntensityChanged);

    gradientStops.push_back(stop);
    updateGradient();
  }
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

bool ofApp::hitTestGradientStop(GradientStop* stop, ofVec2f* test) {
  return (*(test) - stop->pos).length() < GRADIENT_STOP_RADIUS;
}

/**
 * Based on code by Marius Watz. Thanks, Marius!
 * @see http://workshop.evolutionzone.com/2007/09/10/code-2d-line-intersection/
 */
int ofApp::findIntersection(
    float p1x, float p1y, float p2x, float p2y,
    float p3x, float p3y, float p4x, float p4y,
    ofVec2f* result) {
  float xD1,yD1,xD2,yD2,xD3,yD3;
  float dot,deg,len1,len2;
  float ua,ub,div;

  // calculate differences
  xD1=p2x-p1x;
  xD2=p4x-p3x;
  yD1=p2y-p1y;
  yD2=p4y-p3y;
  xD3=p1x-p3x;
  yD3=p1y-p3y;

  // calculate the lengths of the two lines
  len1=sqrt(xD1*xD1+yD1*yD1);
  len2=sqrt(xD2*xD2+yD2*yD2);

  // calculate angle between the two lines.
  dot=(xD1*xD2+yD1*yD2); // dot product
  deg=dot/(len1*len2);

  // if abs(angle)==1 then the lines are parallel,
  // so no intersection is possible
  if(abs(deg)==1) return NULL;

  // find intersection Pt between two lines
  div=yD2*xD1-xD2*yD1;
  ua=(xD2*yD3-yD2*xD3)/div;
  ub=(xD1*yD3-yD1*xD3)/div;

  result->set(p1x+ua*xD1, p1y+ua*yD1);
  return 0;
}

