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
    virtual ~vertex(void) = default;

    // find first edge shared by this vertex and b
    edge_type *shared_edge(const vertex_type *b) const;

    // find first edge pointing from this vertex to b
    edge_type *first_edge_to(const vertex_type *b) const;

    // calculate the neighbour hash function for determining if we should match
    // (coarsen) specified edge
    bool neihash(const edge_type *new_e) const;

#ifdef SSE_INTRINSINC
    void *operator new (size_t size) {
        void *p = _mm_malloc(size, 16);
        if (!p) throw std::bad_alloc ();
        return p;
    }

    void operator delete (void *p) {
        _mm_free(p);
    }
#endif

    static int vertex_id;
    int id;
    vector3d_type x = vector3d_type::zero;
    vector3d_type dx = vector3d_type::zero;
    vector3d_type ddx = vector3d_type::zero;
    vector3d_type ddx_ = vector3d_type::zero;
    vector3d_type delta = vector3d_type::zero;
    vertex *coarser = nullptr;
    std::vector<edge_type *> es;
};

template <typename _coord_type>
int vertex<_coord_type>::vertex_id;


template <typename _coord_type>
class vertex_styled : public vertex<_coord_type>
{
public:
    vertex_styled(_coord_type x, _coord_type y, _coord_type z)
        : vertex<_coord_type>(x, y, z) {
    }
    vertex_styled(const vector3d<_coord_type> &x)
        : vertex<_coord_type>(x) {
    }

    // render this vertex via OpenGL
    void render(void) const;

    shape_type shape = shape_type::cube;
    color_type color = color_type::blue;
    double size = 1.0;
    int shape_detail = 10;
    std::wstring label = L"";
    std::string font_family = "Helvetica";
    color_type font_color = color_type::white;
    int font_size = 16;
    bool visible = true;
};


// 
// Edge type definitions
//   edge: base type for a collection of edge types
//   edge_styled: edges in finest layer, with style info, not reference counted
// 

template <typename _coord_type>
class edge
{
public:
    typedef vertex<_coord_type> vertex_type;
    typedef vector3d<_coord_type> vector3d_type;

    edge(vertex_type *a, vertex_type *b, 
        bool refcounted = true, bool oriented = false)
        : a(a), b(b), cnt(0), 
          refcounted(refcounted), oriented(oriented) {
    }
    edge(const edge &) = delete;
    virtual ~edge(void) = default;

    // Connect/Disconnect the edge from a to b
    edge<_coord_type> *connect(void);
    void disconnect(void);
        
    vertex<_coord_type> * const a;
    vertex<_coord_type> * const b;
    _coord_type strength = 1.0;
    int cnt = 0;
    bool refcounted: 1;
    bool oriented: 1;
};


template <typename _coord_type>
class edge_styled : public edge<_coord_type>
{
public:
    typedef vertex<_coord_type> vertex_type;
    typedef vector3d<_coord_type> vector3d_type;

    edge_styled(vertex_type *a, vertex_type *b)
        : edge<_coord_type>(a, b, false, false),
          visible(true), 
          arrow(false), arrow_reverse(false),
          spline(false), showstrain(false), blendcolor(false) {
    }

    virtual ~edge_styled(void) {
        delete vspline;
    }

    // render this edge via OpenGL
    void render(void) const;

    // set this edge as a spline edge
    void set_spline(bool is_spline = true) {
        spline = is_spline;
        if (is_spline and vspline == nullptr) {
            vspline = new vertex_spline_centroid(this);
        }
        else {
            delete vspline;
        }
    }

    bool visible: 1;
    bool arrow: 1;
    bool arrow_reverse: 1;
    bool spline: 1;
    bool showstrain: 1;
    bool blendcolor: 1;
    double arrow_position = 0.5;
    double arrow_radius = 1.0;
    double arrow_length = 1.0;
    color_type color = color_type::darkgray;
    std::wstring label = L"";
    std::string font_family = "Helvetica";
    color_type font_color = color_type::white;
    int font_size = 16;
    stroke_type stroke = stroke_type::solid;
    double width = 1.0;

    // 
    // centroid position of spline edge. this centroid is represented as a
    // specialized vertex and involves the calculation of force-directed layout in
    // the finest layer
    // 
    class vertex_spline_centroid : public vertex<_coord_type> {
    public:
        vertex_spline_centroid(edge_styled<_coord_type> *e) 
            : vertex<_coord_type>(_coord_type(0.5) * (e->a->x + e->b->x)),
              e_spline(e) {
            this->es.push_back(new edge<_coord_type>(this, e->a, false, false));
            if (e->a != e->b)
                this->es.push_back(
                    new edge<_coord_type>(this, e->b, false, false));
        }
        virtual ~vertex_spline_centroid(void) {
            for (auto e : this->es) delete e;
        }
        edge_styled<_coord_type> *e_spline;
    } *vspline = nullptr;

protected:
    void render_arrow(
        const vector3d_type &arrow_dir, 
        GLfloat ax, GLfloat ay, GLfloat az) const;
};


// Find the first edge shared by this vertex (notated as a) and b (a->b or b->a),
// this is maily for testing if a and b is connected or not
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

// Calculate the value of neighbour hash function, the result is a boolean indicating
// whether we should match (or say, collapse) this edge in coarser graph.
template <typename _coord_type>
bool vertex<_coord_type>::neihash(const edge<_coord_type> *new_e) const
{
    if (new_e->a == new_e->b) return false;
    for (auto e: es) {
        if (e->a != e->b and 
            e->a->coarser == e->b->coarser)
            return false;
            // return (rand() % 3) == 0;
    }
    return true;
}


// Connect vertex a and b. According to if the edge is reference counted, we might
// increase the reference count of an already existing instead of making a hard link
// with this edge object
template <typename _coord_type>
edge<_coord_type> *edge<_coord_type>::connect(void)
{
    assert(cnt >= 0);
    auto e_ = refcounted? 
        a->first_edge_to(b): 
        nullptr;
    if (e_) {
        e_->cnt += 1;
        if (e_ != this) delete this;
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
