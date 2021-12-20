#pragma once

#include <vector>
#include <queue>
#include <chrono>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>

#include "pctl/parray.hpp"
#include "pctl/datapar.hpp"

#include "graph.h"

#define SYNC __atomic_thread_fence(__ATOMIC_SEQ_CST)
#define LOAD(addr) __atomic_load_n(&addr, __ATOMIC_SEQ_CST)
#define STORE(addr, value) __atomic_store_n(&addr, value, __ATOMIC_SEQ_CST)
#define CAS(real, exp, value) __atomic_compare_exchange_n(&real, &exp, value, false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)

#define LOG 0

size_t now() {
    return std::chrono::high_resolution_clock::now().time_since_epoch().count();
}

struct bfs_t {

    const int block;

    bfs_t(int block): block(block) {}

    template<class func_t, class neighbours>
    void seq(int start, const graph<neighbours>& g, func_t&& callback) {
        std::vector<int> colour(g.size(), 0);
        std::vector<int> frontier;
        frontier.push_back(start);
        colour[start] = 1;
        int curDepth = 0;
        while (frontier.size()) {
            std::vector<int> new_frontier;
#if LOG
            double curSize = frontier.size();
            size_t start = now();
#endif
            for (auto cur : frontier) {
                callback(cur, curDepth);
                for (int i = 0; i < g.get_neighbours(cur).size(); i++) {
                    const int n = g.get_neighbours(cur)[i];
                    if (!colour[n]) {
                        colour[n] = 1;
                        new_frontier.push_back(n);
                    }
                }
            }
#if LOG
            size_t fin = now();
            std::cout << "seq: " << curSize << ' ' << (fin - start) << '\n';
#endif
            curDepth++;
            frontier = new_frontier;
        }
    }

    template<class func_t, class neighbours>
    void par(int start, const graph<neighbours>& g, func_t&& callback) {
        pasl::pctl::parray<int8_t> colour(g.size(), static_cast<int8_t>(0));
        pasl::pctl::parray<int> frontier(1, start);
        STORE(colour[start], 1);
        int curDepth = 0;
        while (frontier.size()) {
            if (frontier.size() < block) {
                SYNC;
                std::vector<int> new_frontier;
#if LOG
                double curSize = frontier.size();
                size_t start = now();
#endif
                for (auto cur : frontier) {
                    callback(cur, curDepth);
                    for (int i = 0; i < g.get_neighbours(cur).size(); i++) {
                        const int n = g.get_neighbours(cur)[i];
                        if (!colour[n]) {
                            colour[n] = 1;
                            new_frontier.push_back(n);
                        }
                    }
                }
#if LOG
                size_t fin = now();
                std::cout << "psq: " << curSize << ' ' << (fin - start) << '\n';
#endif
                curDepth++;
                frontier = pasl::pctl::parray<int>(new_frontier.size(), [&](size_t ind) {
                    return new_frontier[ind];
                });
                SYNC;
            } else {
#if LOG
                double curSize = frontier.size();
                size_t start = now();
#endif
                pasl::pctl::parray<int> frontier_d(frontier.size(), [&](int ind) {
                    const int f = frontier[ind];
                    callback(f, curDepth);
                    return g.get_n_neighbours(f);
                });
                curDepth++;
                size_t fin0 = now();

                pasl::pctl::parray<int> frontier_sd = pasl::pctl::scan(frontier_d.begin(), frontier_d.end(), 0, 
                    [&](int a, int b){
                        return a + b;
                    }, pasl::pctl::scan_type::forward_exclusive_scan);
                size_t fin1 = now();
                const int sum = frontier_sd[frontier_sd.size() - 1] + frontier_d[frontier_d.size() - 1];
                pasl::pctl::parray<int> new_frontier(sum, -1);
                size_t fin2 = now();

                pasl::pctl::parallel_for(0l, frontier.size(), [&](size_t ind) {
                    const int f = frontier[ind];
                    const int sd = frontier_sd[ind];
                    for (int i = 0; i < g.get_neighbours(f).size(); i++) {
                        const int n = g.get_neighbours(f)[i];
                        int8_t zero = 0;
                        if (CAS(colour[n], zero, 1)) {
                            new_frontier[sd + i] = n;
                        }
                    }
                });
                size_t fin3 = now();

                frontier = pasl::pctl::filter(new_frontier.begin(), new_frontier.end(), [&](int cur) {
                    return cur != -1;
                });
#if LOG
                size_t fin4 = now();

                double t0 = fin0 - start;
                double t1 = fin1 - fin0;
                double t2 = fin2 - fin1;
                double t3 = fin3 - fin2;
                double t4 = fin4 - fin3;
                double total = fin4 - start;
                // std::cout << t0 / total << ' ' << t1 / total << ' ' << t2 / total << ' ' << t3 / total << ' ' << t4 / total << '\n';
                std::cout << "par: " << curSize << ' ' << total << '\n';
#endif
            }
        }
    }
};
