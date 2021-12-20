#include <iostream>
#include <algorithm>
#include <iterator>
#include <functional>
#include <cmath>
#include <random>
#include <chrono>

#include "bfs.h"
#include "graph.h"

auto get_now() {
    return std::chrono::high_resolution_clock::now();//.time_since_epoch().count();
}

bool checker() {
    bfs_t bfs(10);
    cubic_graph graph(20);
    std::vector<int> res(graph.size());
    int seq_count = 0;
    bfs.seq(0, graph, [&](int v, int d) {
        seq_count++;
        res[v] = d;
    });
    std::atomic<bool> success(true);
    std::atomic<int> count(0);
    bfs.par(0, graph, [&](int v, int d) {
        count++;
        if (d != res[v]) {
            std::cout << v << ": expected " << res[v] << " but found " << d << '\n';
            success = false;
        }
    });
    return success && count == graph.size() && seq_count == count;
}

template<class Func0, class Func1>
double timer(Func0&& s, Func1&& f) {
    s();
    auto start = get_now();
    f();
    auto finish = get_now();
    std::chrono::duration<double, std::milli> diff = finish - start;
    return diff.count();
}

template<class Func0, class Func1>
double timerN(size_t count, Func0&& s, Func1&& f) {
    double sum = 0;
    for (size_t i = 0; i < count; i++) {
        sum += timer(std::forward<Func0>(s), std::forward<Func1>(f));
    }
    return sum / count;
}

int main() {
    if (checker()) {
        std::cout << "checked\n";
        const int side = 500;
        cubic_graph graph(side);
        bfs_t bfs(00000);
        auto setup = []{};

        std::cout << "seq...\n";
        auto seq = timerN(5, setup, [&]{
            bfs.seq(0, graph, [](int, int){});
        });
        std::cout << "par...\n";
        auto par = timerN(5, setup, [&]{
            bfs.par(0, graph, [](int, int){});
        });
        std::cout << "seq: " << seq << "ms \t";
        std::cout << "par: " << par << "ms \t";
        std::cout << "boost: " << seq / par << "x\n";
    } else {
        std::cout << "failed\n";
    }
    return 0;
}
