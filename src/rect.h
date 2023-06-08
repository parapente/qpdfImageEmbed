#ifndef RECT_H
#define RECT_H

class Rect {
private:
  double x1, x2, y1, y2;

public:
  Rect();
  ~Rect();
  void setCoords(double x1, double y1, double x2, double y2);
  double height();
  double width();
  double x();
  double y();
};

#endif // RECT_H