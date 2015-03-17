
#include "GradientAnimation.h"

GradientAnimation::GradientAnimation() {
  numStops = 0;

  currFrame = NULL;
  currFrameAllocated = false;
}

GradientAnimation::~GradientAnimation() {
  if (currFrameAllocated) {
    delete currFrame;
  }
}

void GradientAnimation::setup(int w, int h) {
  width = w;
  height = h;

  GradientKeyframe* keyframe;

  keyframe = new GradientKeyframe;
  keyframe->setup(width, height);
  keyframes.push_back(keyframe);

  keyframe = new GradientKeyframe;
  keyframe->setup(width, height);
  keyframes.push_back(keyframe);

  currFrame = keyframes[0];
  currFrameAllocated = false;
}

void GradientAnimation::draw() {
  if (currFrame) {
    currFrame->draw();
  }
}

void GradientAnimation::render(unsigned short int* pixelsDetail, unsigned char* pixels) {
  if (currFrame) {
    currFrame->render(pixelsDetail, pixels);
  }
}

void GradientAnimation::clearKeyframes() {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    delete keyframes[i];
  }
}

GradientKeyframe* GradientAnimation::getKeyframe(int i) {
  return keyframes[i];
}

int GradientAnimation::numKeyframes() {
  return keyframes.size();
}

void GradientAnimation::addStop(int x, int y) {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    GradientKeyframe* keyframe = keyframes[i];
    GradientStop* stop = keyframe->addStop(
        "stop " + ofToString(numStops), x, y,
        numStops % 2 == 0 ? 0 : 65025);
  }
  numStops++;
}

void GradientAnimation::clearStops() {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    GradientKeyframe* keyframe = keyframes[i];
    keyframe->clearStops();
  }
  numStops = 0;
}

float GradientAnimation::getTime() {
  return time;
}

void GradientAnimation::setTime(float t) {
  time = t;

  if (currFrameAllocated) {
    delete currFrame;
  }

  if (t <= 0) {
    currFrame = keyframes[0];
    currFrameAllocated = false;
  }
  else if (t >= 1) {
    currFrame = keyframes[keyframes.size() - 1];
    currFrameAllocated = false;
  }
  else {
    currFrame = new GradientFrame;
    currFrame->setup(width, height);
    for (int i = 0; i < numStops; i++) {
      currFrame->addStop(getInterpolatedStop(i, t));
    }
    currFrameAllocated = true;
  }
}

GradientStop* GradientAnimation::getInterpolatedStop(int i, float t) {
  GradientStop* stop = new GradientStop;

  // FIXME: Gradient stop interpolation currently assumes only two keyframes.
  GradientStop* first = keyframes[0]->getStop(i);
  GradientStop* last = keyframes[keyframes.size()-1]->getStop(i);

  stop->pos.set(
      ofMap(t, 0, 1, first->pos.x, last->pos.x),
      ofMap(t, 0, 1, first->pos.y, last->pos.y));
  stop->dir.set(
      ofMap(t, 0, 1, first->dir.x, last->dir.x),
      ofMap(t, 0, 1, first->dir.y, last->dir.y));

  stop->intensity = new ofxIntSlider;
  *(stop->intensity) = (int)ofMap(t, 0, 1, *(first->intensity), *(last->intensity));

  return stop;
}

