#ifndef _LAYER_H_
#define _LAYER_H_


#include "vertex_edge.hh"
#include <queue>
#include <set>

// #define debuglog(...) {  printf(__VA_ARGS__); printf("\n");  }
#define debuglog(...)

template <typename _container_ty, typename _elem_ty>
bool contains(const _container_ty &c, const _elem_ty &e) {
	// return std::find(c.begin(), c.end(), e) != c.end();
	return c.find(e) != c.end();
}


// 
// A layer of graph. Both finest layers and coarsen layers are of this type.  Each
// layer has a `coarser` member for accessing the coarser version of the same
// graph. 
// 
// Lagrange dynamics are calculated from the coarsest layer all the way down to the
// finest layer, with some multilevel dynamics mechanism for transfering the layout
// of coarser layer to the finer layer
// 

template <typename _coord_type>
class layer
{
public:
	typedef _coord_type float_type;
	typedef vector3d<_coord_type> vector3d_type;
	typedef layer<_coord_type> layer_type;
	typedef vertex<_coord_type> vertex_type;
	typedef edge<_coord_type> edge_type;

	layer(double f0, double K, double eps, double damping, double dilation)
		: f0(f0), K(K), eps(eps), damping(damping), dilation(dilation) {
	}
	layer(const layer &) = delete;

	void add_vertex(vertex_type *v)
	{
		vs.push_back(v);
		if (coarser) {
			vertex_type *cv = new vertex_type(v->x);
			debuglog("add_vertex: add_vertex (coarser): %d", cv->id);
			coarser->add_vertex(cv);
			v->coarser = cv;
		}
	}

	void remove_vertex(vertex_type *v)
	{
		assert(v->es.size() == 0);
		if (coarser) {
			coarser->remove_vertex(v->coarser);
            delete v->coarser;
		}
		vs.erase(std::find(vs.begin(), vs.end(), v));
	}

	edge_type *add_edge(edge_type *e)
	{
		vertex_type *a = e->a, *b = e->b;
		bool matched = a->neihash(e) and b->neihash(e);
		edge_type *ret = e->connect();
		if (coarser) {
			vertex_type *ca = a->coarser, *cb = b->coarser;
			edge_type *e_new = new edge_type(ca, cb);
			debuglog("add_edge: add_edge (coarser): %d -> %d", ca->id, cb->id);
			e_new = coarser->add_edge(e_new);
			if (matched and ca != cb) {
				debuglog("calling match on %d(%d) and %d(%d)", a->id, ca->id, b->id, cb->id);
				match(a, b);
			}
		}
		return ret;
	}

	void remove_edge(edge_type *e)
	{
		vertex_type *a = e->a, *b = e->b;
		e->disconnect();
		bool aeb_connected = (a->shared_edge(b) != nullptr);
		if (coarser) {
			vertex_type *ca = a->coarser, *cb = b->coarser;
			debuglog("remove_edge: remove_edge (coarser): %d -> %d", ca->id, cb->id);
			coarser->remove_edge(ca->shared_edge(cb));
			if (a != b and ca == cb and !aeb_connected) {
				debuglog("removing collapsed edge (%d -> %d), might need to split", a->id, b->id);
				split(a, b);
			}
		}
	}

	void layout(float_type dt);

protected:
	void match(vertex_type *a, vertex_type *b)
	{
		vertex_type *ca = a->coarser, *cb = b->coarser;
		assert(ca != cb);
		auto es = cb->es;
		for (auto e : es) {
			edge_type *e_new = 
				(e->a == e->b)? new edge_type(ca, ca):
				(e->a == cb)? new edge_type(ca, e->b):
				new edge_type(e->a, ca);
			int cnt = e->cnt;
			for (int k = 0; k < cnt; k++) {
				debuglog("match: remove_edge: %d -> %d", e->a->id, e->b->id);
				coarser->remove_edge(e);
				debuglog("match: add_edge: %d -> %d", e_new->a->id, e_new->b->id);
				e_new = coarser->add_edge(e_new);
			}
		}

		auto b_comp = matched_component(b);
		for (auto v : b_comp) v->coarser = ca;
		coarser->remove_vertex(cb);
		delete cb;
	}

	void split(vertex_type *a, vertex_type *b)
	{
		vertex_type *ca = a->coarser, *cb = b->coarser;
		assert(ca == cb and a != b);
		auto split_nodes = matched_component(b);
		if (contains(split_nodes, a)) {
			return;				// a and b are still in the same matched component,
								// don't need to split
		}
		
		vertex_type *new_cb = new vertex_type(b->x);
		debuglog("split: add_vertex: %d", new_cb->id);
		coarser->add_vertex(new_cb);

		assert(!split_nodes.empty());
		for (auto v : split_nodes) {
			for (auto e : v->es) {
				vertex_type *cv = nullptr;
				edge_type *e_new = nullptr;
				if (contains(split_nodes, e->a) and contains(split_nodes, e->b)) {
					assert(e->a->coarser == e->b->coarser);
					if (e->a == v) {
						cv = e->b->coarser;
						e_new = new edge_type(new_cb, new_cb);
					}
					else {
						continue; // the counter side would handle this
					}
				}
				else if (e->a == v) {
					cv = e->b->coarser;
					e_new = new edge_type(new_cb, cv);
				}
				else {
					cv = e->a->coarser;
					e_new = new edge_type(cv, new_cb);
				}

				int cnt = e->cnt;
				for (int k = 0; k < cnt; k++) {
					edge_type *e_old = ca->shared_edge(cv);
					debuglog("split: remove_edge (e_old: %d -> %d)", e_old->a->id, e_old->b->id);
					coarser->remove_edge(e_old);
					debuglog("split: add_edge (e_new: %d -> %d)" , e_new->a->id, e_new->b->id);
					e_new = coarser->add_edge(e_new);
				}
			}
		}

		for (auto v : split_nodes) {
			v->coarser = new_cb;
		}
	}		

	std::set<vertex_type *> matched_component(vertex_type *c)
	{
		std::set<vertex_type *> matched_comp;
		std::queue<vertex_type *> qvs;
        qvs.push(c);
		while (!qvs.empty()) {
			vertex_type *v = qvs.front(); qvs.pop();
			matched_comp.insert(v);
			for (auto e : v->es) {
				if (e->a != e->b and e->a->coarser == e->b->coarser) {
					vertex_type *vv = (v == e->b)? e->a: e->b;
					if (!contains(matched_comp, vv)) qvs.push(vv);
				}
			}
		}
		return matched_comp;
	}


    // for debugging
    int nd_edges(void)
    {
        int n = 0;
        for (auto v : vs) {
            for (auto e : v->es) {
                n += (e->a == e->b? 2: 1) * e->cnt;
            }
        }
        return n;
    }

public:
    bool verify_integrity(void)
    {
        if (!coarser) return true;
        for (auto v : vs) {
            for (auto e : v->es) {
                vertex_type *a = e->a, *b = e->b;
                vertex_type *ca = a->coarser, *cb = b->coarser;
                if (ca->shared_edge(cb) == nullptr)
                    return false;
            }
        }
        return nd_edges() == coarser->nd_edges();
    }

    bool verify_redundancy(void)
    {
        if (!coarser) return true;
        for (auto cv : coarser->vs) {
            bool redundant = true;
            for (auto v : vs) {
                if (v->coarser == cv) {
                    redundant = false;
                    break;
                }
            }
            if (redundant) return false;
        }
        return true;
    }

public:
	std::vector<vertex_type *> vs;
	layer_type *coarser = nullptr;

protected:
	float_type f0;				// repulsion factor
	float_type K;				// spring factor
	float_type eps;				// small constant to get rid of division singularity
	float_type damping;         // damping factor for dissipating energy
	float_type dilation;		// dilation factor used when transfering the dynamics
								// of coarser graph to the finer graph
};


#endif /* _LAYER_H_ */
