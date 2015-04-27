#ifndef _VERTEX_EDGE_H_
#define _VERTEX_EDGE_H_

#include "vec3d.hh"
#include "prop.hh"
#include <algorithm>
#include <vector>
#include <string>
#include <assert.h>


template <typename _coord_type>
class edge;


// 
// Vertex type definitions
//   vertex: base type for a collection of vertex types
//   vertex_coarse: vertex in coarsen layer, no style info needed
//   vertex_styled: vertex in finest layer, with style info for rendering
// 

template <typename _coord_type>
class vertex
{
public:
    typedef vector3d<_coord_type> vector3d_type;
    typedef vertex<_coord_type> vertex_type;
    typedef edge<_coord_type> edge_type;

    vertex(_coord_type x, _coord_type y, _coord_type z)
        : id(vertex_id++),
          x(x, y, z) {
    }
    vertex(const vector3d_type &x)
        : id(vertex_id++),
		  x(x) {
    }
    vertex(const vertex &) = delete;

    // find first edge shared by this vertex and b
    edge_type *shared_edge(const vertex_type *b) const;

    // find first edge pointing from this vertex to b
    edge_type *first_edge_to(const vertex_type *b) const;

    // calculate the neighbour hash function for determining if we should match
    // (coarsen) specified edge
    bool neihash(const edge_type *new_e) const;

    static int vertex_id;
    int id;
    vector3d_type x = vector3d_type::zero;
    vector3d_type dx = vector3d_type::zero;
    vector3d_type ddx = vector3d_type::zero;
    vector3d_type delta = vector3d_type::zero;
    vertex *coarser = nullptr;
    std::vector<edge_type *> es;
};

template <typename _coord_type>
int vertex<_coord_type>::vertex_id;

template <typename _coord_type>
class vertex_coarse : public vertex<_coord_type> {
public:
    vertex_coarse(_coord_type x, _coord_type y, _coord_type z)
        : vertex<_coord_type>(x, y, z) {
    }
};

template <typename _coord_type>
class vertex_styled : public vertex<_coord_type>
{
public:
    vertex_styled(_coord_type x, _coord_type y, _coord_type z)
        : vertex<_coord_type>(x, y, z) {
    }

    shape_type shape = shape_type::cube;
    color_type color = color_type::blue;
    double size = 1.0;
    int shape_detail = 10;
    std::string label = "";
    std::string font_family = "Helvetica";
    color_type font_color = color_type::white;
    int font_size = 12;
    bool visible = true;
};


// 
// Edge type definitions
//   edge: base type for a collection of edge types
//   edge_coarse: edges in coarsen layer, no style info, reference counted
//   edge_styled: edges in finest layer, with style info, not reference counted
// 

template <typename _coord_type>
class edge
{
public:
    typedef vertex<_coord_type> vertex_type;
    edge(vertex_type *a, vertex_type *b, 
        bool refcounted = true, bool oriented = false)
        : a(a), b(b), cnt(0), refcounted(refcounted), oriented(oriented) {
    }
    edge(const edge &) = delete;

    // Connect/Disconnect the edge from a to b
    edge<_coord_type> *connect(void);
    void disconnect(void);
        
    vertex<_coord_type> *a;
    vertex<_coord_type> *b;
    double strength = 1.0;
    int cnt = 0;
    bool refcounted: 1;
    bool oriented: 1;
};

template <typename _coord_type>
class edge_coarse : public edge<_coord_type> {
public:
    typedef vertex<_coord_type> vertex_type;
    edge_coarse(vertex_type *a, vertex_type *b)
        : edge<_coord_type>(a, b, true, false) {
    }
};

template <typename _coord_type>
class edge_styled : public edge<_coord_type>
{
public:
    typedef vertex<_coord_type> vertex_type;
    edge_styled(vertex_type *a, vertex_type *b)
        : edge<_coord_type>(a, b, false, false),
          visible(true), 
          arrow(false), arrow_reverse(false),
          spline(false), showstrain(false) {
    }

    bool visible: 1;
    bool arrow: 1;
    bool arrow_reverse: 1;
    bool spline: 1;
    bool showstrain: 1;
    double arrow_position = 0.5;
    double arrow_radius = 1.0;
    double arrow_length = 1.0;
    color_type color = color_type::blue;
    std::string label = "";
    std::string font_family = "Helvetica";
    color_type font_color = color_type::white;
    int font_size = 12;
    stroke_type stroke = stroke_type::solid;
    double width = 1.0;
};


// Find the first edge shared by this vertex (notated as a) and b (a->b or
// b->a), this is maily for testing if a and b is connected or not
template <typename _coord_type>
edge<_coord_type> *
vertex<_coord_type>::shared_edge(const vertex<_coord_type> *b) const
{
    for (auto e: es) {
        if ((e->a == this and e->b == b) or
            (e->b == this and e->a == b)) return e;
    }
    return nullptr;
}

// Find the first edge pointing from this vertex outward to b
template <typename _coord_type>
edge<_coord_type> *
vertex<_coord_type>::first_edge_to(const vertex<_coord_type> *b) const
{
    for (auto e: es) {
        if (e->a == this and e->b == b) return e;
    }
    return nullptr;
}

// Calculate the value of neighbour hash function, the result is a boolean
// indicating whether we should match (or say, collapse) this edge in coarser
// graph.
template <typename _coord_type>
bool vertex<_coord_type>::neihash(const edge<_coord_type> *new_e) const
{
    if (new_e->a == new_e->b) return false;
    for (auto e: es) {
        if (e->a != e->b and 
            e->a->coarser == e->b->coarser) return false;
    }
    return true;
}


// Connect vertex a and b. According to if the edge is reference counted, we
// might increase the reference count of an already existing instead of making
// a hard link with this edge object
template <typename _coord_type>
edge<_coord_type> *edge<_coord_type>::connect(void)
{
    assert(cnt >= 0);
    auto e_ = refcounted? 
        a->first_edge_to(b): 
        nullptr;
    if (e_) {
        e_->cnt += 1;
        delete this;
        return e_;
    }
    else {
        a->es.push_back(this);
        if (a != b) b->es.push_back(this);
        cnt += 1;
        return this;
    }
}

// Disconnect a from b
template <typename _coord_type>
void edge<_coord_type>::disconnect(void)
{
    assert(cnt > 0);
    cnt -= 1;
    if (cnt == 0) {
        a->es.erase(std::find(a->es.begin(), a->es.end(), this));
        if (a != b) {
            b->es.erase(std::find(b->es.begin(), b->es.end(), this));
        }
        delete this;
    }
}


#endif /* _VERTEX_EDGE_H_ */
