#include <utility>

struct Rect;

struct Shape {
    virtual ~Shape() = default;
    virtual int area() const = 0;
};

struct Rect : public Shape {
    Rect() = default;
    Rect(int width, int height) : width(width), height(height) {}
    int width;
    int height;
    int area() const override;
};

int Rect::area() const {
    return this->width * this->height;
}

bool operator==(const Rect& left, const Rect& right) {
    return left.width == right.width && left.height == right.height;
}

bool sameShape(const Rect& left, const Rect& right);

bool sameShape(const Rect& left, const Rect& right) {
    return left == right;
}

int main() {
    Rect a = Rect(3, 4);
    Rect b = Rect(3, 4);
    if (sameShape(a, std::move(b))) {
        return a.area();
    }
    return 0;
}

