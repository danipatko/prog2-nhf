#ifndef ALGO_H
#define ALGO_H

#include "consts.h"
#include "diagnostics.h"
#include "lib.h"
#include "util.h"

#include <algorithm>
#include <queue>
#include <stack>
#include <vector>

template <typename T> struct Weight {
    virtual float get(const T &from, const T &to, const T *prev = nullptr) const = 0;

    float get(int from, int to, int prev, const DiGraph<T> &graph) const {
        return get(graph.at(from), graph.at(to), prev < 0 ? nullptr : &graph.at(prev));
    }

    virtual ~Weight() = default;
};

template <typename T> struct Algorithm : Counter, virtual Sizable {
  protected:
    const DiGraph<T> &graph;
    std::vector<int> prev;

  public:
    enum class Driver {
        Dijkstra,
        AStar,
        BFS,
        DFS,
    };

    class Trace : Sizable {
        std::vector<int> trace;
        unsigned int _current = 0, index = 0;

      public:
        size_t size_of() const override {
            return true_size(trace) + sizeof(unsigned int) * 2;
        }

        /**
         * Marks this node as the parent of the following nodes
         */
        Trace &parent(unsigned int index) {
            if (trace.size() > 0)
                trace.push_back(-1);

            trace.push_back(index);
            return *this;
        }

        /**
         * Add as children
         */
        Trace &child(unsigned int index) {
            trace.push_back(index);
            return *this;
        }

        /**
         * Clear the records, reset counter
         */
        void reset() {
            trace.clear();
            _current = 0;
        }

        /**
         * @brief first item (parent) of the current segment
         */
        int current() {
            return trace[_current];
        }

        /**
         * @brief skip one item within this segment
         */
        int next() {
            return trace[index++];
        }

        /**
         * @brief advance to the next segment
         */
        void skip() {
            index++;
            _current = index;
        }

        /**
         * @brief checks bounds and current segment
         */
        bool has_next() {
            return index < trace.size() - 1 && trace[index] >= 0;
        }

        /**
         * @brief checks bounds
         */
        bool consumed() {
            return index >= trace.size() - 4;
        }
    };

  public:
    Trace trace;

    Algorithm(const DiGraph<T> &graph) : graph(graph), prev(graph.size(), -1) {}

    virtual void run(int source, int target, bool break_on_found = false) = 0;

    size_t size_of() const override {
        return true_size(prev) + trace.size_of();
    }

    /**
     * @brief reconstruct the path, after the algorithm finished.
     */
    virtual std::vector<int> reconstruct(int source, int target) const {
        std::vector<int> path;
        int u = target;

        while (u != source) {
            if (u < 0) {
                // TODO: handle this
                std::cout << "No route to point\n";
                return path;
            }

            path.push_back(u);
            u = this->prev[u];
        }

        path.push_back(source);

        std::reverse(path.begin(), path.end());
        return path;
    }

    virtual ~Algorithm() = default;
};

template <typename T> class Dijkstra : public Algorithm<T> {
    const Weight<T> &weight;

    // for storing weight and index
    using PQitem = std::pair<float, int>;

    std::vector<float> distance;
    std::vector<bool> visited;
    std::priority_queue<PQitem, std::vector<PQitem>, std::greater<PQitem>> pq; // max-heap

  public:
    Dijkstra(const DiGraph<T> &graph, const Weight<T> &weight)
        : Algorithm<T>(graph), weight(weight), //
          distance(graph.size(), FMAX), visited(graph.size(), false) {
        this->mem(graph.size() * 2);
    }

    size_t size_of() const override {
        return Algorithm<T>::size_of() + true_size(distance) + true_size(visited) //
               + sizeof(pq) + sizeof(std::vector<PQitem>) + sizeof(PQitem) * pq.size() * 2;
    }

    void run(int source, int target, bool break_on_found = false) override {
        distance[source] = 0.f;
        pq.emplace(0.f, source);
        bool found = false;
        this->mem(3);

        while (!found && !pq.empty()) {
            const float d = pq.top().first;
            const int current = pq.top().second;
            pq.pop();
            this->mem(2);

            this->comp();
            if (visited[current])
                continue;

            visited[current] = true;
            this->mem();

            this->trace.parent(current);

            for (int neighbor : this->graph.adjacent(current)) {
                this->trace.child(neighbor);
                this->step();

                this->mem();
                const float w = this->weight.get(current, neighbor, this->prev[current], this->graph);

                this->comp();
                if (d + w < distance[neighbor]) {
                    distance[neighbor] = d + w;
                    this->prev[neighbor] = current;

                    pq.emplace(d + w, neighbor);
                    this->mem(3);
                }

                this->comp();
                if (break_on_found && neighbor == target) {
                    found = true;
                    break;
                }
            }
        }
    }
};

template <typename T> class AStar : public Algorithm<T> {
  private:
    const Weight<T> &heuristic;
    const Weight<T> &weight;

    using PQitem = std::pair<float, int>; // (f_score, node)
    std::priority_queue<PQitem, std::vector<PQitem>, std::greater<PQitem>> open_set;

    std::vector<float> g_score;

  public:
    AStar(const DiGraph<T> &graph, const Weight<T> &weight, const Weight<T> &heuristic)
        :                                                            //
          Algorithm<T>(graph), weight(weight), heuristic(heuristic), //
          g_score(graph.size(), -1) {}

    size_t size_of() const override {
        return Algorithm<T>::size_of() + true_size(g_score) //
               + sizeof(open_set) + sizeof(std::vector<PQitem>) + sizeof(PQitem) * open_set.size() * 2;
    }

    void run(int source, int target, bool break_on_found = false) override {
        open_set.push(PQitem{this->heuristic.get(source, target, source, this->graph), source});
        g_score[source] = 0.0f;
        this->mem(2);

        while (!open_set.empty()) {
            int current = open_set.top().second;
            open_set.pop();
            this->mem();

            this->comp();
            if (break_on_found && current == target) {
                return;
            }

            this->trace.parent(current);
            for (int neighbor : this->graph.adjacent(current)) {
                this->step();

                const float w = this->weight.get(current, neighbor, this->prev[current], this->graph);
                const float tentative_g = g_score[current] + w;
                this->mem(2);

                this->comp();
                if (g_score[neighbor] < 0 || tentative_g < g_score[neighbor]) {
                    this->trace.child(neighbor);

                    this->prev[neighbor] = current;
                    g_score[neighbor] = tentative_g;

                    const float f_score = tentative_g + this->heuristic.get(source, target, this->prev[current], this->graph);
                    open_set.push({f_score, neighbor});

                    this->mem(3);
                }
            }
        }
    }
};

template <typename T> class BFS : public Algorithm<T> {
  private:
    std::vector<bool> visited;
    std::queue<int> queue;

  public:
    BFS(const DiGraph<T> &graph)
        : Algorithm<T>(graph), //
          visited(graph.size(), false) {}

    size_t size_of() const override {
        return Algorithm<T>::size_of() + true_size(visited) + sizeof(queue) + sizeof(T) * queue.size();
    }

    void run(int source, int target, bool break_on_found = false) override {
        queue.push(source);
        visited[source] = true;

        this->mem(2);

        while (!queue.empty()) {
            int current = queue.front();
            queue.pop();
            this->mem();

            this->trace.parent(current);

            this->comp(2); // this if and the while check
            if (break_on_found && current == target) {
                break;
            }

            for (int neighbor : this->graph.adjacent(current)) {
                this->step();

                this->comp();
                if (!visited[neighbor]) {
                    this->trace.child(neighbor);

                    visited[neighbor] = true;
                    this->prev[neighbor] = current;

                    queue.push(neighbor);

                    this->mem(3);
                }
            }
        }
    }
};

template <typename T> class DFS : public Algorithm<T> {
  private:
    std::vector<bool> visited;
    std::stack<int> stack;

  public:
    DFS(const DiGraph<T> &graph)
        : Algorithm<T>(graph), //
          visited(graph.size(), false) {}

    size_t size_of() const override {
        return Algorithm<T>::size_of() + true_size(visited) + sizeof(stack) + stack.size() * sizeof(T);
    }

    void run(int source, int target, bool break_on_found = false) override {
        stack.push(source);
        visited[source] = true;
        this->mem(2);

        while (!stack.empty()) {
            const int current = stack.top();
            stack.pop();
            this->mem();

            this->trace.parent(current);

            this->comp(2); // while statement and this
            if (break_on_found && current == target)
                break;

            for (int neighbor : this->graph.adjacent(current)) {
                this->step();

                this->comp();
                if (!visited[neighbor]) {
                    this->trace.child(neighbor);

                    visited[neighbor] = true;
                    this->prev[neighbor] = current;

                    stack.push(neighbor);

                    this->mem(3);
                }
            }
        }
    }
};

#endif // ALGO_H