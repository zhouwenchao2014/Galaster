#ifndef _PROP_H_
#define _PROP_H_

#include <stdint.h>

enum class shape_type {
    sphere,
    cube,
    octahedron,
    dodecahedron,
    icosahedron,
    rhombicdodecahedron,
    cone,
    torus
};

enum class stroke_type {
    solid,
    dotted,
    dashed,
    none
};

class color_type {
public:
    color_type(uint8_t r, uint8_t g, uint8_t b)
        : r(r * recp_255), g(g * recp_255), b(b * recp_255) {
    }
    color_type(int r, int g, int b)
        : r(r * recp_255), g(g * recp_255), b(b * recp_255) {
    }
    color_type(double r, double g, double b)
        : r(r), g(g), b(b) {
    }

    double r, g, b;
    constexpr static const double recp_255 = 1.0 / 255.0;

    double redd(void) const { return r; }
    double greend(void) const { return g; }
    double blued(void) const { return b; }

    static const color_type red, green, blue, white;
};

const color_type color_type::red(255, 0, 0);
const color_type color_type::green(0, 255, 0);
const color_type color_type::blue(0, 0, 255);
const color_type color_type::white(255, 255, 255);


#endif /* _PROP_H_ */
