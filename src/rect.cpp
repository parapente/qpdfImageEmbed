#include "rect.h"
#include <cmath>

Rect::Rect() { x1 = x2 = y1 = y2 = 0; }

Rect::~Rect() {}

double Rect::height() { return std::fabs(y2 - y1); }

double Rect::width() { return std::fabs(x2 - x1); }

void Rect::setCoords(double x1, double y1, double x2, double y2) {
  this->x1 = x1;
  this->x2 = x2;
  this->y1 = y1;
  this->y2 = y2;
}

double Rect::x() {
  if (x1 < x2) {
    return x1;
  }

  return x2;
}

double Rect::y() {
  if (y1 < y2) {
    return y1;
  }

  return y2;
}