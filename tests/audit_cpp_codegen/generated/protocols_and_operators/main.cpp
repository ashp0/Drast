#include "runtime/drast_runtime.hpp"

struct Rect;

struct Shape {
    virtual ~Shape() = default;
    virtual int area() = 0;
};

struct Rect : public Shape {
    Rect() = default;
    Rect(int width, int height) : width(width), height(height) {}
    int width;
    int height;
    int area();
};

int Rect::area() {
    return this->width * this->height;
}

bool operator==(const Rect& left, const Rect& right) {
    return left.width == right.width && left.height == right.height;
}

bool sameShape(const Rect& left, const Rect& right);

bool sameShape(const Rect& left, const Rect& right) {
    return left == right;
}

int main(int argc, char **argv) {
    drast::setArgs(argc, argv);
    Rect a = Rect(3, 4);
    Rect b = Rect(3, 4);
    if (sameShape(a, b)) {
        return a.area();
    }
    return 0;
}

