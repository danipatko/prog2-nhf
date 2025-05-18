#ifndef GRAPH_H
#define GRAPH_H

#include "diagnostics.h"
#include "util.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <vector>

template <typename T> struct Vertex {
    T data;
    int idx;

    Vertex(T data, int index = -1) : data(data), idx(index) {};
};

/// This is required for hashmap to work
namespace std {
template <typename T> struct equal_to<Vertex<T>> {
    constexpr bool operator()(const Vertex<T> &lhs, const Vertex<T> &rhs) const {
        return (lhs.idx == rhs.idx);
    }
};

template <typename T> struct hash<Vertex<T>> {
    size_t operator()(const Vertex<T> &r) const {
        const std::hash<int> int_hash_fn;
        const size_t result = 47 ^ int_hash_fn(r.idx);
        return result;
    }
};

}; // namespace std

/**
 * Abstract class for providing a backend for graph representation
 */
template <typename T> class GraphRepresentation : virtual Sizable {
  protected:
    /**
     * All the vertices contained by this graph.
     */
    std::vector<Vertex<T>> vertices;

  public:
    GraphRepresentation(std::vector<Vertex<T>> vertices) : vertices(vertices) {};

    size_t size_of() const override {
        return true_size(vertices);
    }

    std::vector<Vertex<T>> &vtx() {
        return vertices;
    }

    const std::vector<Vertex<T>> &vtx() const {
        return vertices;
    }

    /**
     * @returns number of vertices
     */
    size_t size() const {
        return vertices.size();
    }

    /**
     * Checks if a Vertex is contained within this graph
     * @param vtx the Vertex to check
     * @returns true if present
     */
    bool contains(int vtx) const {
        return vtx >= 0 && vtx < vertices.size();
    }

    /**
     * Get the neighbors of v
     * @returns list of vertices
     */
    virtual std::vector<int> adjacent(int idx) const = 0;

    /**
     * Add an edge to the graph
     * @returns self, for chainability
     */
    virtual GraphRepresentation &edge(int from, int to) = 0;

    /**
     * Add a bi-directional edge to the graph
     * @returns self, for chainability
     */
    virtual GraphRepresentation &b_edge(int a, int b) = 0;

    virtual ~GraphRepresentation() {};
};

/**
 * Adjacency matrix representation
 */
template <typename T> class MGraph : public GraphRepresentation<T>, virtual Sizable {
    using GraphRepresentation<T>::vertices;

    /**
     * N*N matrix
     */
    int **matrix;

    // std::vector<std::vector<int>> mat;

  public:
    MGraph(std::vector<Vertex<T>> vtx) : GraphRepresentation<T>(vtx) {
        matrix = new int *[vtx.size()];

        for (size_t i = 0; i < vtx.size(); i++) {
            // std::cout << i << ". filling...\n";
            matrix[i] = new int[vtx.size()];
            std::fill(matrix[i], matrix[i] + vtx.size(), -1);
        }
    };

    size_t size_of() const override {
        return GraphRepresentation<T>::size_of() + sizeof(int) * vertices.size() * vertices.size();
    }

    std::vector<int> adjacent(int vtx) const override {
        std::vector<int> neighbors;

        for (size_t i = 0; i < vertices.size(); i++)
            if (matrix[vtx][i] >= 0)
                neighbors.push_back(matrix[vtx][i]);

        return neighbors;
    }

    MGraph &edge(int from, int to) override {
        matrix[from][to] = to;
        return *this;
    }

    MGraph &b_edge(int a, int b) override {
        matrix[a][b] = b;
        matrix[b][a] = a;
        return *this;
    }

    ~MGraph() {
        for (size_t i = 0; i < vertices.size(); i++)
            delete[] matrix[i];
        delete[] matrix;
    };
};

/**
 * Adjacency list representation
 */
template <typename T> class LGraph : public GraphRepresentation<T>, virtual Sizable {
    using GraphRepresentation<T>::vertices;

    /**
     * An N long list
     */
    std::unordered_set<int> *edges;

  public:
    LGraph(std::vector<Vertex<T>> vlist) : GraphRepresentation<T>(vlist) {
        edges = new std::unordered_set<int>[vlist.size()];
    };

    size_t size_of() const override {
        size_t set_size = 0;

        for (int i = 0; i < vertices.size(); i++)
            set_size += true_size(edges[i]);

        return GraphRepresentation<T>::size_of() + set_size;
    }

    std::vector<int> adjacent(int v) const override {
        std::vector<int> neighbors;

        for (int r : edges[v]) {
            neighbors.push_back(r);
        }

        return neighbors;
    }

    LGraph &edge(int from, int to) override {
        edges[from].insert(to);
        return *this;
    }

    LGraph &b_edge(int from, int to) override {
        return edge(from, to).edge(to, from);
    }

    ~LGraph() {
        delete[] edges;
    };
};

// ----

template <typename T> class DiGraph : Sizable {
  public:
    enum class Driver { Matrix, List };
    Driver driver;

  private:
    GraphRepresentation<T> *G;

  public:
    DiGraph(std::vector<Vertex<T>> vlist, Driver driver = Driver::List) : driver(driver) {
        switch (driver) {
        case Driver::Matrix:
            G = new MGraph<T>(vlist);
            break;

        case Driver::List:
            G = new LGraph<T>(vlist);
            break;

        default:
            throw std::invalid_argument("invalid graph driver!");
        }
    }

    size_t size_of() const override {
        return G->size_of();
    }

    std::vector<Vertex<T>> &vtx() {
        return G->vtx();
    }

    const std::vector<Vertex<T>> &vtx() const {
        return G->vtx();
    }

    size_t size() const {
        return G->vtx().size();
    }

    const T &at(unsigned int idx) const {
        return G->vtx()[idx].data;
    }

    bool contains(int v) const {
        return G->contains(v);
    }

    std::vector<int> adjacent(int v) const {
        return G->adjacent(v);
    }

    DiGraph &edge(int from, int to) {
        G->edge(from, to);
        return *this;
    }

    DiGraph &b_edge(int from, int to) {
        G->b_edge(from, to);
        return *this;
    }

    ~DiGraph() {
        delete G;
    }
};

#endif // GRAPH_H