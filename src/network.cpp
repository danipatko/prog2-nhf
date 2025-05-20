#include "network.h"
#include "cli.h"
#include "diagnostics.h"
#include "geo.h"
#include "lib.h"

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <vector>

glm::vec3 motor2color(const HighwayType highway) {
    switch (highway) {
    case HighwayType::motorway:
    case HighwayType::motorway_link:
        return gfx::color::ORANGE;

    case HighwayType::primary:
    case HighwayType::primary_link:
        return gfx::color::YELLOW;

    case HighwayType::secondary:
    case HighwayType::secondary_link:
        return gfx::color::GREEN;

    case HighwayType::trunk:
    case HighwayType::trunk_link:
    case HighwayType::tertiary:
    case HighwayType::tertiary_link:
        return gfx::color::CYAN;

    // basic roads
    case HighwayType::unclassified:
    case HighwayType::residential:
    case HighwayType::service:
    case HighwayType::living_street:
    case HighwayType::road:
        return gfx::color::WHITE;

    // Non-motorized paths
    case HighwayType::pedestrian:
    case HighwayType::footway:
    case HighwayType::cycleway:
    case HighwayType::path:
    case HighwayType::bridleway:
    case HighwayType::steps:
        return gfx::color::DARK_GRAY;

    default:
        return gfx::color::WHITE;
    }
}

// ----

void Network::setup() {
    BBox bbox = BBox::max();

    for (int from = 0; from < graph.vtx().size(); from++) {
        for (int to : graph.adjacent(from)) {
            if (from < to) {
                const Node &node_to = graph.at(to), node_from = graph.at(from);
                bbox.include(node_to);
                map.geo_roads.add(transform(node_from), transform(node_to), motor2color(node_from.road->highway), node_from.road->visibility());
            }
        }
    }

    map.panzoom.set(map.project(transform(bbox.center())));
    map.panzoom.set(2.f / bbox.w);

    map.geo_roads.update();
}

/**
 * @brief trace down the visited nodes
 * @returns true if the trace has been consumed
 */
bool Network::trace(Algorithm<Node>::Trace &trace, const int spread_rate) {
    if (trace.consumed())
        return true;

    for (int i = 0; i < spread_rate; i++) {
        while (trace.has_next()) {
            const auto p1 = transform(graph.at(trace.current()));
            const auto p2 = transform(graph.at(trace.next()));

            map.discovered.add(p1, p2, color::ORANGERED, graph.at(trace.current()).road->rating() * 2);
        }

        trace.skip();
    }

    map.discovered.update();
    return false;
}

/**
 * @brief renders a new section of the route path
 * @returns true if the path has been fully traversed
 */
bool Network::route(const std::vector<int> &path, const int route_rate) {
    static int index = 1;

    if (index >= path.size())
        return true;

    for (int i = 0; index < path.size() && i < route_rate; i++) {
        const auto p1 = transform(graph.at(path[index - 1])), //
            p2 = transform(graph.at(path[index]));

        index++;
        map.route.add(p1, p2, color::CYAN, 9999);
    }

    map.route.update();
    return false;
}

void Network::run(Algorithm<Node> &algo, const std::vector<int> &path, const int source, const int target, const cli::Options &options) {
    map.points.add(transform(graph.at(target)));
    map.points.add(transform(graph.at(source)));
    map.points.update();

    // references for capture
    Algorithm<Node>::Trace &trace = algo.trace;
    const int &trace_rate = options.trace_rate;
    const int &route_rate = options.route_rate;

    map.callback = [this, &trace, &path, &trace_rate, &route_rate]() {
        switch (state) {
        case Stage::Trace:
            if (!this->trace(trace, trace_rate))
                break;

            map.dim_roads(0.6);
            map.dim_discovered(0.6);
            state = Stage::Route;

            break;

        case Stage::Route:
            if (this->route(path, route_rate))
                state = Stage::Idle;
            break;

        default: // Stage::Idle:
            return;
        }
    };

    map.route.update();
    map.loop();
}

namespace loader {

bool exists(const std::string &filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

void parse(const std::string &filename, std::vector<Road *> &roads) {
    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open()) {
        std::cerr << "failed to open '" << filename << "'\n";
        exit(EXIT_FAILURE);
    }

    Bench b;

    static std::string line;
    while (std::getline(file, line)) {
        Road *road = new Road;
        road->parse(line);
        roads.push_back(road);

        // user feedback
        if (roads.size() % 1000 == 0) {
            auto el = b.elapsed(true);
            std::cout << "read " << std::setw(6) << roads.size() << " records in " << std::setprecision(6) << el << "ms \tbatch average: " << std::setprecision(6) << roads.size() / el * 1000 << " records/sec\n";
        }
    }

    file.close();
}

std::vector<Road *> from_file(const std::string &filename, bool use_cache) {
    Bench t;

    const std::string cached_name = filename + ".cache.bin";

    std::vector<Road *> roads;

    if (use_cache && exists(cached_name)) {
        Serializable::read(cached_name.c_str(), roads);

    } else {
        parse(filename, roads);

        if (use_cache) {
            Serializable::write(cached_name.c_str(), roads);
        }
    }

    return roads;
}

//

DiGraph<Node> construct_graph(std::vector<Vertex<Node>> &vlist, const std::vector<unsigned int> &segments, const cli::Options &options) {
    if (options.graph == DiGraph<Node>::Driver::Matrix) {
        std::cout << "You appear to have chosen the adjacency matrix graph driver. "
                     "This data structure is highly inefficient memory-wise.\n"
                  << "The graph contains " << vlist.size() << " vertices, and the matrix size is about to be: " << vlist.size() * vlist.size() * sizeof(int) / 1024.f / 1024.f << " MBs\n"
                  << "Do you wish to continue? [yn] ";

        char c;
        std::cin >> c;

        if (c != 'y') {
            std::cerr << "aborted\n";
            exit(EXIT_FAILURE);
        }
    }

    DiGraph<Node> graph(vlist, options.graph);

    // STEP 1: connect segments
    for (size_t i = 1, counter = 0; i < vlist.size(); i++) {
        // don't connect edge
        if (counter < segments.size() && segments[counter] == i) {
            // check if we were at the end of a roundabout
            // if so, connect the start of this segment with the previous point
            if (vlist[i].data.road->roundabout) {
                graph.edge(counter - 1 < 0 ? 0 : segments[counter - 1],
                           i - 1); // (roundabouts are always oneway)
            }

            // on to the next segment
            counter++;
            continue;
        }

        if (vlist[i].data.road->oneway) {
            graph.edge(i - 1, i);
        } else {
            graph.b_edge(i - 1, i);
        }
    }

    // STEP 2: connect overlapping roads
    // concept: make the points hashable, keep track of the Nodes sharing the same
    // Points using a hashmap. This way only one iteration is required over the
    // vertex list (note: hashing might have minimal inaccuracy, luckily it's
    // irrelevant for out purpose)

    std::unordered_map<size_t, std::vector<Vertex<Node> *>> point_map;

    for (int i = 0; i < vlist.size(); i++) {
        const size_t key = vlist[i].data.loc->hash();

        if (point_map.count(key) == 0)
            point_map.insert({key, std::vector<Vertex<Node> *>(1, &vlist[i])});
        else
            point_map[key].push_back(&vlist[i]);
    }

    for (auto &p : point_map) {
        for (int i = 1; i < p.second.size(); i++) {
            // sanity check the distance
            if (Point::within(p.second[i - 1]->data, p.second[i]->data, 1.f)) {
                // TOFIX:
                // check for overpasses (either connecting 2 bridge components or
                // non-bridge ones) the following statement results in bridges not being
                // connected if (p.second[i - 1]->data.road->bridge ==
                // p.second[i]->data.road->bridge) { }

                graph.b_edge(p.second[i - 1]->idx, p.second[i]->idx);
            }
        }
    }

    // ----
    // I'm going to leave this here for future reference
    // seemed promising, however failed to correctly connect like 5%
    // of the vertices it should've. still no idea why...
    //
    // STEP 2: connect overlapping roads
    // idea: greedy algorithm to sort the points by distance from origin (or
    // centroid) points of the same coordinates should be next to each other in
    // the vector
    // ----

    // std::sort(vlist.begin(), vlist.end(),
    //           [origin](const Vertex<Node> &a, const Vertex<Node> &b) {
    //               return Point::distance_sq(*a.data.loc, origin) <
    //               Point::distance_sq(*b.data.loc, origin);
    //           });

    // for (int i = 0; /* i < 100 && */ i < vlist.size() - 1; i++) {
    //     if (Point::within(*vlist[i].data.loc, *vlist[i + 1].data.loc, 1.f)) {
    //         // check for overpasses! (either connecting 2 bridge components or
    //         non-bridge ones) if (vlist[i].data.road->bridge == vlist[i +
    //         1].data.road->bridge) {
    //             graph.b_edge(vlist[i].idx, vlist[i + 1].idx);
    //         }
    //     }
    // }

    return graph;
}

DiGraph<Node> construct(const std::vector<Road *> &roads, const cli::Options &options) {
    std::vector<unsigned int> segments;
    std::vector<Vertex<Node>> vlist;

    for (Road *road : roads) {
        for (Point *p : road->coordinates)
            vlist.push_back(Vertex<Node>(Node(road, p), vlist.size()));

        if (vlist.size() > 0)
            segments.push_back(vlist.size());
    }

    return construct_graph(vlist, segments, options);
}

}; // namespace loader
