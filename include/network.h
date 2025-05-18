#ifndef NETWORK_H
#define NETWORK_H

#include "algorithm.h"
#include "cli.h"
#include "geo.h"
#include "map.h"

#include <string>
#include <vector>

class Network {
  private:
    Map map;

    const DiGraph<Node> &graph;
    const std::vector<Road *> &roads;

    glm::vec2 transform(const Point &p, const Point &translate = Point()) {
        return {p.x, p.y};
    }

    enum class Stage {
        Trace,
        Route,
        Idle,
    } state = Stage::Trace;

    /**
     * @brief add edges to graphics buffer, set panzoom to fit the middle of points
     */
    void setup();

    /**
     * @brief trace down the visited nodes
     * @returns true if the trace has been consumed
     */
    bool trace(Algorithm<Node>::Trace &trace, const int spread_rate = 1000);

    /**
     * @brief renders a new section of the route path
     * @returns true if the path has been fully traversed
     */
    bool route(const std::vector<int> &path, const int route_rate = 20);

  public:
    Network(const DiGraph<Node> &_graph, const std::vector<Road *> &_roads) : graph(_graph), roads(_roads) {
        setup();
    }

    /**
     * @brief starts the GUI part of the app
     * steps
     * 1. renders the map
     * 2. animates the trace of the alogrithm
     * 3. then animates the route from source to target
     */
    void run(Algorithm<Node> &algo, const std::vector<int> &path, const int source, const int target, const cli::Options &options);

    ~Network() {
        for (Road *p : roads)
            delete p;
    }
};

namespace loader {

std::vector<Road *> from_file(const std::string &filename, bool use_cache = true);

DiGraph<Node> construct(const std::vector<Road *> &roads, const cli::Options &opts);

}; // namespace loader

#endif // NETWORK_H