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

enum class renderer {
    solid, particle
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

    unsigned int c4u(unsigned int alpha = 255) const {
        unsigned int rgba;
        ((unsigned char *) &rgba)[0] = redd() * 255;
        ((unsigned char *) &rgba)[1] = greend() * 255;
        ((unsigned char *) &rgba)[2] = blued() * 255;
        ((unsigned char *) &rgba)[3] = alpha;
        return rgba;
    }

    static const color_type red, green, blue, white, darkgray;
};


#endif /* _PROP_H_ */
