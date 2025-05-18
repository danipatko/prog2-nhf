#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <chrono>
#include <cstddef>
#include <iostream>

class Bench {
    using clock = std::chrono::high_resolution_clock;

    clock::time_point t1, t2;
    std::string TAG;

  public:
    Bench(const std::string &tag = "timer") : t1(clock::now()), TAG(std::move(tag)) {}

    void start() {
        t1 = clock::now();
    }

    void stop() {
        t2 = clock::now();
    }

    /**
     * @brief time elapsed since last start call (in milliseconds)
     */
    double elapsed(bool now = false) {
        if (now)
            t2 = clock::now();

        std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        return ms_double.count();
    }

    void eval(bool now = false) {
        std::cout << TAG << " finished in: " << elapsed(now) << "ms\n";
    }
};

struct Sizable {
    virtual size_t size_of() const = 0;
};

struct Counter {
    /**
     * @brief number of steps
     */
    unsigned int steps;

    /**
     * @brief number of memory operations
     */
    unsigned int memops;

    /**
     * @brief number of comparisons
     */
    unsigned int comparisons;

    Counter() : steps(0), memops(0), comparisons(0) {};

    void reset() {
        steps = 0;
    }

    /**
     * @brief Increase number of steps
     */
    void step(unsigned int t = 1) {
        steps += t;
    }

    /**
     * @brief Increase number of memory operations.
     * General rule: every assignment counts as one!
     */
    void mem(unsigned int t = 1) {
        memops += t;
    }

    /**
     * @brief Increase number of comparisons (basically if statements).
     */
    void comp(unsigned int t = 1) {
        comparisons += t;
    }
};

#endif