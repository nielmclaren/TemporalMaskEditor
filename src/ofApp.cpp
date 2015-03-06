#include "ofApp.h"

void ofApp::setup() {
  brushImage.loadImage("brushes/001-soft.png");
  brushImage.setImageType(OF_IMAGE_GRAYSCALE);

  screenWidth = ofGetWindowWidth();
  screenHeight = ofGetWindowHeight();

  frameWidth = 0;
  frameHeight = 0;
  frameCount = 0;

  brushColor = 0;
  brushDeltaRemainder = 0;

  loadSettings();
  updateBrush();

  isPreviewDragging = false;

  inputPixels = NULL;
  maskPixels = NULL;
  maskPixelsDetail = NULL;
  outputPixels = NULL;

  gui.setup();
  gui.add(brushFlow.setup("brush flow", 130, 10, 2000));
  gui.add(brushSize.setup("brush size", 100, 10, 300));
  gui.add(brushStep.setup("brush step", 10, 1, 20));

  // TODO: Make these exclusive. There's a problem listening to toggle events.
  gui.add(brushButton.setup("brush", false));
  gui.add(gradientButton.setup("gradient", true));
  drawMode = LINEAR_GRADIENT_DRAW_MODE;

  loadFrames("adam_magyar_stainless01");

  gradientStartX = 100;
  gradientStartY = 200;
  gradientEndX = 500;
  gradientEndY = 400;
  drawGradient();
}

void ofApp::update() {
  if (frameWidth > 0) {
    // Delay before playback
    if (ofGetSystemTime() - keyDownTime > 400) {
      updatePreviewIndex(previewIndexDelta);
    }
  }
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

    preview.draw(0, 0, frameWidth/2, frameHeight/2);

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

    if (mouseY > frameHeight/2) {
      ofEnableAlphaBlending();
      ofNoFill();
      ofSetColor(255, 255, 255, 64);
      ofSetLineWidth(2);
      ofCircle(mouseX, mouseY, brushSize/2);
      ofCircle(frameWidth/2 + mouseX/2, (mouseY - frameHeight/2) / 2, brushSize/4);
      ofDisableAlphaBlending();
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
  brushFlow = settings.getValue("settings:brushFlow", 128);
  brushSize = settings.getValue("settings:brushSize", 100);
  brushStep = settings.getValue("settings:brushStep", 10);
}

void ofApp::saveSettings() {
  ofxXmlSettings settings;
  settings.setValue("settings:brushFlow", brushFlow);
  settings.setValue("settings:brushSize", brushSize);
  settings.setValue("settings:brushStep", brushStep);
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

  previewIndex = 0;

  frameCount = 0;
  frameWidth = 0;
  frameHeight = 0;

  frameToBrushColor = 0;
  maxColor = 0;
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
  previewPixels = new unsigned char[frameWidth * frameHeight * 3];
  outputPixels = new unsigned char[frameWidth * frameHeight * 3];

  clearMask();

  frameToBrushColor = 255 * 255 / (frameCount - 1);
  brushColor = maxColor = (frameCount - 1) * frameToBrushColor;
  previewIndex = frameCount - 1;

  cout << "Loading complete." << endl;
}

void ofApp::clearMask() {
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    maskPixels[i] = maskPixelsDetail[i] = 0;
  }
}

void ofApp::loadMask() {
  if (mask.loadImage("mask.png")) {
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

void ofApp::updatePreviewIndex(int delta) {
  previewIndexDelta = delta;
  setPreviewIndex(previewIndex + delta);
}

void ofApp::setPreviewIndex(int i) {
  previewIndex = i;

  if (previewIndex < 0) {
    previewIndex = 0;
  }
  if (previewIndex >= frameCount) {
    previewIndex = frameCount - 1;
  }

  brushColor = previewIndex * frameToBrushColor;
  for (int i = 0; i < frameWidth * frameHeight; i++) {
    for (int c = 0; c < 3; c++) {
      previewPixels[i * 3 + c] = inputPixels[previewIndex * frameWidth * frameHeight * 3 + i * 3 + c];
    }
  }
  preview.setFromPixels(previewPixels, frameWidth, frameHeight, OF_IMAGE_COLOR);
}

void ofApp::updateBrush() {
  brush.clone(brushImage);
  brush.setImageType(OF_IMAGE_GRAYSCALE);
  brush.resize(brushSize, brushSize);
  brushPixels = brush.getPixels();
}

void ofApp::addPoint(float startX, float startY, bool newStroke) {
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

void ofApp::addBrush(int tx, int ty) {
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
          maskPixelsDetail[pix] += delta / abs(delta) * MIN(brushFlow * brushPixels[tPix] / 255.0, abs(delta));
          maskPixels[pix] = maskPixelsDetail[pix] / 255;
        }
      }
      tPix++;
    }
    tPix += offSetCorrectionRight;
  }
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
        maskPixelsDetail[i] = 0;
      }
      else {
        maskPixelsDetail[i] = CLAMP(w.length() / d, 0, 1) * 255  *  255;
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

void ofApp::keyReleased(int key) {
  switch (key) {
    case OF_KEY_RIGHT:
      updatePreviewIndex(0);
      break;

    case OF_KEY_LEFT:
      updatePreviewIndex(0);
      break;

    case OF_KEY_UP:
      brushFlow = MIN(255 * 255, brushFlow + 1);
      cout << "Brush flow: " << brushFlow << endl;
      break;

    case OF_KEY_DOWN:
      brushFlow = MAX(0, brushFlow - 1);
      cout << "Brush flow: " << brushFlow << endl;
      break;

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

    case 'v':
      brushSize = brushSize + 5;
      cout << "Brush size: " << brushSize << endl;
      updateBrush();
      break;

    case 'c':
      brushSize = brushSize - 5;
      cout << "Brush size: " << brushSize << endl;
      updateBrush();
      break;
  }
}

void ofApp::mouseMoved(int x, int y) {
}

void ofApp::mouseDragged(int x, int y, int button) {
  if (y < frameHeight/2) {
    if (isPreviewDragging) {
      setPreviewIndex((float) x / (frameWidth/2) * (frameCount - 2));
    }
  }
  else {
    switch (drawMode) {
      case BRUSH_DRAW_MODE:
        addPoint(x, y - frameHeight/2, true);
        break;
      case LINEAR_GRADIENT_DRAW_MODE:
        break;
      case RADIAL_GRADIENT_DRAW_MODE:
        break;
    }
  }
}

void ofApp::mousePressed(int x, int y, int button) {
  if (y < frameHeight/2 && x < frameWidth/2) {
    setPreviewIndex((float) x / (frameWidth/2) * (frameCount - 2));
    isPreviewDragging = true;
  }
  else {
    switch (drawMode) {
      case BRUSH_DRAW_MODE:
        addPoint(x, y - frameHeight/2, true);
        break;
      case LINEAR_GRADIENT_DRAW_MODE:
      case RADIAL_GRADIENT_DRAW_MODE:
        gradientStartX = x;
        gradientStartY = y - frameHeight/2;
        break;
    }
  }
}

void ofApp::mouseReleased(int x, int y, int button) {
  isPreviewDragging = false;
  switch (drawMode) {
    case BRUSH_DRAW_MODE:
      break;
    case LINEAR_GRADIENT_DRAW_MODE:
    case RADIAL_GRADIENT_DRAW_MODE:
      gradientEndX = x;
      gradientEndY = y - frameHeight/2;
      drawGradient();
      break;
  }
}

void ofApp::windowResized(int w, int h) {
}

void ofApp::gotMessage(ofMessage msg) {
}

void ofApp::dragEvent(ofDragInfo dragInfo) {
}

