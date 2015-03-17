#include "GradientKeyframe.h"

GradientKeyframe::GradientKeyframe() {
}

GradientKeyframe::~GradientKeyframe() {
}

void GradientKeyframe::draw() {
  for (int i = 0; i < stops.size(); i++) {
    GradientStop* stop = stops[i];
    ofVec2f d = stop->dir.getScaled(15);
    ofCircle(stop->pos.x, stop->pos.y, GRADIENT_STOP_RADIUS);
    ofCircle(stop->pos.x, stop->pos.y, GRADIENT_STOP_RADIUS * 0.75);
    ofLine(stop->pos.x, stop->pos.y, stop->pos.x + d.x, stop->pos.y + d.y);
  }
}

