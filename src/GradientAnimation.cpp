
#include "GradientAnimation.h"

GradientAnimation::GradientAnimation() {
}

GradientAnimation::~GradientAnimation() {
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
    int numStops = keyframe->numStops();
    GradientStop* stop = keyframe->addStop(
        "stop " + ofToString(numStops), x, y,
        numStops % 2 == 0 ? 0 : 65025);
  }
}

void GradientAnimation::clearStops() {
  int numKeyframes = keyframes.size();
  for (int i = 0; i < numKeyframes; i++) {
    GradientKeyframe* keyframe = keyframes[i];
    keyframe->clearStops();
  }
}

