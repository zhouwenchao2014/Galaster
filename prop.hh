#ifndef _PROP_H_
#define _PROP_H_

#include <stdint.h>

enum class shape_type {
    sphere,
    cube,
    octahedron,
    dodecahedron,
    icosahedron,
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
        : r(r), g(g), b(b) {
    }
    uint8_t r, g, b;

    double redd(void) const { return r / 255.0; }
    double greend(void) const { return g / 255.0; }
    double blued(void) const { return b / 255.0; }

    static const color_type red, green, blue, white;
};

const color_type color_type::red(255, 0, 0);
const color_type color_type::green(0, 255, 0);
const color_type color_type::blue(0, 0, 255);
const color_type color_type::white(255, 255, 255);


#endif /* _PROP_H_ */
