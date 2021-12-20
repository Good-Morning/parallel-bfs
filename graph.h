#pragma once

#include <array>
#include <tuple>

template<class N>
class graph {
public:
    class neighbours {
        virtual size_t size() const = 0;
        virtual int operator[](int ind) const = 0;
    };

    virtual size_t size() const = 0;
    virtual size_t get_n_neighbours(int i) const = 0;
    virtual N get_neighbours(int i) const = 0;
};

class cubic_neighbours : public graph<cubic_neighbours>::neighbours {
    size_t s = 0;
    std::array<int, 6> data;

public:
    size_t size() const override;
    int operator[](int ind) const override;
    inline void push(int v) {
        data[s++] = v;
    }
};

class cubic_graph : public graph<cubic_neighbours> {

    const int side;
    const int side2;
    const int side3;

public:
    size_t size() const override;
    size_t get_n_neighbours(int i) const override;
    cubic_neighbours get_neighbours(int i) const override;

    cubic_graph(int side)
    : side(side), side2(side * side), side3(side2 * side) {}

private:
    inline bool check(int x) const {
        return x > 0 && x < side - 1;
    }

    inline std::tuple<int, int, int> get_coords(int ind) const {
        const int z = ind / side2;
        const int y = ind / side - z * side;
        const int x = ind - z * side2 - y * side;
        return {x, y, z};
    }
};
