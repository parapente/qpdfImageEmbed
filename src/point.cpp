#include "point.h"

Point::Point() {
    m_x = 0;
    m_y = 0;
}

Point::Point(float x, float y) {
    m_x = x;
    m_y = y;
}

float Point::x() { return m_x; }

float Point::y() { return m_y; }