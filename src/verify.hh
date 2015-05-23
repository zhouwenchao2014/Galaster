#ifndef _VERIFY_H_
#define _VERIFY_H_


#include "graph.hh"


// for debugging
template <typename _coord_type>
int nd_edges(layer<_coord_type> *layer)
{
    int n = 0;
    for (auto v : layer->vs) {
        for (auto e : v->es) {
            n += (e->a == e->b? 2: 1) * e->cnt;
        }
    }
    return n;
}


template <typename _coord_type>
bool verify_integrity(layer<_coord_type> *layer)
{
    if (!layer->coarser) return true;
    for (auto v : layer->vs) {
        for (auto e : v->es) {
            vertex<_coord_type> *a = e->a, *b = e->b;
            vertex<_coord_type> *ca = a->coarser, *cb = b->coarser;
            if (ca->shared_edge(cb) == nullptr)
                return false;
        }
    }
    return nd_edges(layer) == nd_edges(layer->coarser);
}

template <typename _coord_type>
bool verify_integrity(graph<_coord_type> *graph)
{
    for (auto layer : graph->layers) {
        if (!verify_integrity(layer)) return false;
    }
    return true;
}


template <typename _coord_type>
bool verify_redundancy(layer<_coord_type> *layer)
{
    if (!layer->coarser) return true;
    for (auto cv : layer->coarser->vs) {
        bool redundant = true;
        for (auto v : layer->vs) {
            if (v->coarser == cv) {
                redundant = false;
                break;
            }
        }
        if (redundant) return false;
    }
    return true;
}

template <typename _coord_type>
bool verify_redundancy(graph<_coord_type> *graph)
{
    for (auto layer : graph->layers) {
        if (!verify_redundancy(layer)) return false;
    }
    return true;
}


template <typename _coord_type>
void dump_graphviz(const layer<_coord_type> *layer, const std::string &filename)
{
    FILE *fp = fopen(filename.c_str(), "w+");
    fprintf(fp, "digraph G {\n");
    fprintf(fp, "node [ shape = \"circle\" ];\n");
    for (auto v : layer->vs) {
        fprintf(fp, "  v%d [ label = \"v%d\" ];\n", v->id, v->id);
        for (auto e : v->es) {
            if (e->a == v) {
                if (e->a != e->b and 
                    e->a->coarser == e->b->coarser and
                    e->a->coarser != nullptr) {
                    fprintf(fp, "  v%d -> v%d [ penwidth = 4 ];\n", 
                        e->a->id, e->b->id);
                }
                else {
                    fprintf(fp, "  v%d -> v%d [ penwidth = 1 ];\n", 
                        e->a->id, e->b->id);
                }
            }
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);
}


#endif /* _VERIFY_H_ */
