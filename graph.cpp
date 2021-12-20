#include "graph.h"

size_t cubic_neighbours::size() const {
    return s;
}

int cubic_neighbours::operator[](int ind) const {
    return data[ind];
}

size_t cubic_graph::size() const {
    return side3;
}

size_t cubic_graph::get_n_neighbours(int i) const {
    auto [x, y, z] = get_coords(i); 
    return 3 + check(x) + check(y) + check(z);
}

cubic_neighbours cubic_graph::get_neighbours(int i) const {
    auto [x, y, z] = get_coords(i);
    bool bx = check(x);
    bool by = check(y);
    bool bz = check(z);
    cubic_neighbours res;
    if (bx && by && bz) {
        res.push(i - 1);
        res.push(i + 1);
        res.push(i - side);
        res.push(i + side);
        res.push(i - side2);
        res.push(i + side2);
    } else {
        if (x > 0) { res.push(i - 1); }
        if (y > 0) { res.push(i - side); }
        if (z > 0) { res.push(i - side2); }
        if (x < side - 1) { res.push(i + 1); }
        if (y < side - 1) { res.push(i + side); }
        if (z < side - 1) { res.push(i + side2); }
    }
    return res;
}
