#include "GradientFrame.h"

GradientFrame::~GradientFrame() {
  int numStops = stops.size();
  for (int i = 0; i < numStops; i++) {
    delete stops[i];
  }
}

void GradientFrame::setup(int w, int h) {
  width = w;
  height = h;
}

void GradientFrame::draw() {
  for (int i = 0; i < stops.size(); i++) {
    GradientStop* stop = stops[i];
    ofVec2f d = stop->dir.getScaled(15);
    ofCircle(stop->pos.x, stop->pos.y, GRADIENT_STOP_RADIUS);
    ofLine(stop->pos.x, stop->pos.y, stop->pos.x + d.x, stop->pos.y + d.y);
  }
}

void GradientFrame::render(
    unsigned short int* pixelsDetail,
    unsigned char* pixels) {
  int numStops = stops.size();
  ofVec2f d;

  for (int i = 0; i < width * height; i++) {
    pixelsDetail[i] = pixels[i] = 0;
  }

  if (numStops < 2) return;

  for (int i = 0; i < numStops - 1; i++) {
    GradientStop* stop0 = stops[i];
    GradientStop* stop1 = stops[i + 1];

    updateGradient(pixelsDetail, pixels, stop0, stop1);
  }
}

GradientStop* GradientFrame::addStop(string label, float posx, float posy, int intensity) {
  return addStop(label, posx, posy, 0, 0, intensity);
}

GradientStop* GradientFrame::addStop(string label, float posx, float posy, float dirx, float diry, int intensity) {
  int numStops = stops.size();

  GradientStop* stop = new GradientStop;
  stop->pos.set(posx, posy);
  stop->dir.set(dirx, diry);

  ofxIntSlider* slider = new ofxIntSlider;
  slider->setup(label, intensity, 0, 65025);
  stop->intensity = slider;

  return addStop(stop);
}

GradientStop* GradientFrame::addStop(GradientStop* stop) {
  stops.push_back(stop);
  updateStopDirs();
  return stop;
}

GradientStop* GradientFrame::getStop(int stopIndex) {
  return stops[stopIndex];
}

int GradientFrame::numStops() {
  return stops.size();
}

void GradientFrame::clearStops() {
  int numStops = stops.size();
  for (int i = 0; i < numStops; i++) {
    delete stops[i];
  }
  stops.clear();
}

void GradientFrame::updateStopDirs() {
  int numStops = stops.size();

  for (int i = 0; i < numStops; i++) {
    stops[i]->dir.zero();
  }

  if (numStops < 2) return;

  for (int i = 0; i < numStops; i++) {
    GradientStop* curr = stops[i];

    if (i > 0) {
      GradientStop* prev = stops[i - 1];
      curr->dir += curr->pos - prev->pos;
    }
    if (i < numStops - 1) {
      GradientStop* next = stops[i + 1];
      curr->dir += next->pos - curr->pos;
    }

    curr->dir.normalize();
  }
}

void GradientFrame::updateGradient(
    unsigned short int* pixelsDetail,
    unsigned char* pixels,
    GradientStop* stop0,
    GradientStop* stop1) {
  ofVec2f p0 = stop0->pos;
  ofVec2f p1 = stop1->pos;
  ofVec2f perp0(-stop0->dir.y, stop0->dir.x);
  ofVec2f perp1(-stop1->dir.y, stop1->dir.x);
  ofVec2f curr;
  ofVec2f u, w, v = p1 - p0;
  float d = v.length();
  float b;
  float value;
  ofVec2f intersect0, intersect1;
  float intersectDist, intersectDist0, intersectDist1;
  int i;

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      i = y * width + x;
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
          *(stop0->intensity), *(stop1->intensity),
          true);
        pixelsDetail[i] = MAX(pixelsDetail[i], value);
      }

      pixels[i] = pixelsDetail[i] / 255;
    }
  }
}

GradientStop* GradientFrame::hitTestStops(int x, int y) {
  ofVec2f mouse(x, y);

  int numStops = stops.size();
  for (int i = 0; i < numStops; i++) {
    GradientStop* stop = stops[i];
    if (hitTestStop(stop, &mouse)) {
      return stops[i];
    }
  }
  return NULL;
}

bool GradientFrame::hitTestStop(GradientStop* stop, ofVec2f* test) {
  return (*(test) - stop->pos).length() < GRADIENT_STOP_RADIUS;
}

/**
 * Based on code by Marius Watz. Thanks, Marius!
 * @see http://workshop.evolutionzone.com/2007/09/10/code-2d-line-intersection/
 */
int GradientFrame::findIntersection(
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

