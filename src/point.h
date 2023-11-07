#ifndef POINT_H

class Point {
    private:
        float m_x;
        float m_y;

    public:
        Point();
        Point(float x, float y);

        float x();
        float y();
};

#endif // POINT_H