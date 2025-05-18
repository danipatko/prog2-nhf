#ifndef CLI_H
#define CLI_H

#include "algorithm.h"
#include "geo.h"
#include "lib.h"
#include "util.h"
#include "weights.h"

#include <cstdlib>
#include <cstring>
#include <string>

namespace cli {

static const char *HELPMSG = R"(
Usage:
  --start <location>
        Specifies the starting point. Two formats are accepted:
        - A coordinate like `47.4733817,19.0572901`
        - DMS format, e.g. `47°27'18.9"N 19°07'33.1"E`
        In both cases, the program finds the closest point on the map and starts the route from there.

  --destination <location>
        Specifies the destination. Works the same way as the starting point.

  --map <path/to/map.geojsonl>
        Loads the map. Expected format: newline-delimited GeoJSON (GeoJSONL).
        Tip: Many major cities are available for download here: https://app.interline.io/osm_extracts/interactive_view

  --algo <astar|dijkstra|bfs|dfs>
        Specifies the graph traversal algorithm to use. Supported algorithms:
        - A* Search
        - Dijkstra's Algorithm
        - Breadth-First Search (BFS)
        - Depth-First Search (DFS)

  --struct <list|matrix>
        Chooses the data structure for representing the graph: adjacency list or adjacency matrix.

  --trace-rate <ticks/sec>
        Sets the animation speed for discovered edges.

  --route-rate <ticks/sec>
        Sets the animation speed for the planned route.

  --route <shortest|fastest>
        Defines how edge weights are computed:
        - shortest: Finds the shortest distance.
        - fastest: Fastest in time - considers speed limits and road data (if available in the map).

  --config <a,b,c,d,e,f>
        Defines custom weight multipliers for the algorithm. All values are floats, separated by either ',' or '|'.
        - a: inverse road speed multiplier. can be used to penalyze slow roads
        - b: time multiplier (in seconds)
        - c: multiplier for distance of the two points (in metres)
        - d: multiplier for turn angles (angle is between 0 and PI3 radians (120deg), sharper turns = more penalty)    
        - e: penalty for roads that are not for cars
        - f: multiplier for road ratings. base roads have a penalty of 64, interstates 1 (scaled exponentially)
        - g: penalty for toll roads.

  --help
        Displays this help message.
)";

struct Options {
    /**
     * @brief soure and target nodes of the search
     */
    Point source, target;

    /**
     * @brief animation speed for tracing (red lines) and routing (the cyan one)
     */
    unsigned int trace_rate, route_rate;

    /**
     * @brief map to load (.geojsonl file)
     */
    std::string map;

    /**
     * @brief Directed Graph structure: List, or Matrix
     */
    DiGraph<Node>::Driver graph;

    /**
     * @brief Algorithm to use
     */
    Algorithm<Node>::Driver algorithm;

    /**
     * @brief Routing options: can be Fastest, Shortest or Custom
     */
    RouteOpt routing;

    /**
     * @brief Routing coefficients for Custom routing
     */
    Coefficients *coeffs = nullptr;

    ~Options() {
        if (coeffs != nullptr)
            delete coeffs;
    }
};

/// https://stackoverflow.com/a/46711735
constexpr uint32_t hash(const char *data, size_t size) noexcept {
    uint32_t hash = 5381;

    for (const char *c = data; c < data + size; ++c)
        hash = ((hash << 5) + hash) + (unsigned char)*c;

    return hash;
}

inline void check(int argc, int i) {
    if (i >= argc) {
        std::cerr << "Missing positional argument!\n";
        exit(EXIT_FAILURE);
    }
}

inline Options parse(int argc, char *argv[]) {
    Options opts = {
        .source = 0,
        .target = 0,
        .trace_rate = 1000,
        .route_rate = 10,
        .map = "data/budapest.roads.geojsonl",
        .graph = DiGraph<Node>::Driver::List,
        .algorithm = Algorithm<Node>::Driver::AStar,
        .routing = RouteOpt::Custom,
        .coeffs = nullptr,
    };

    for (int i = 1; i < argc; i++) {
        switch (hash(argv[i], strlen(argv[i]))) {
        case hash("-s", 2):
        case hash("--start", 7):
        case hash("--source", 8):
            check(argc, i + 1);
            opts.source = Point::parse(argv[++i]);
            break;

        case hash("-d", 2):
        case hash("-t", 2):
        case hash("--dest", 6):
        case hash("--destination", 13):
        case hash("--target", 8):
            check(argc, i + 1);
            opts.target = Point::parse(argv[++i]);
            break;

        case hash("-m", 2):
        case hash("--map", 5):
            check(argc, i + 1);
            opts.map = std::string(argv[++i]);
            break;

        case hash("-a", 2):
        case hash("--algo", 6):
            check(argc, i + 1);

            if (!strcmp(argv[i + 1], "astar"))
                opts.algorithm = Algorithm<Node>::Driver::AStar;
            else if (!strcmp(argv[i + 1], "dijkstra"))
                opts.algorithm = Algorithm<Node>::Driver::Dijkstra;
            else if (!strcmp(argv[i + 1], "bfs"))
                opts.algorithm = Algorithm<Node>::Driver::BFS;
            else if (!strcmp(argv[i + 1], "dfs"))
                opts.algorithm = Algorithm<Node>::Driver::DFS;
            else {
                std::cerr << "Invalid algorithm driver '" << argv[i + 1] << "'\n"
                          << "Valid options are: astar, dijkstra, bfs, dfs\n";
                exit(EXIT_FAILURE);
            }

            i++;
            break;

        case hash("--route-rate", 12):
            check(argc, i + 1);
            opts.route_rate = Parser::as_stream<int>(argv[++i]);
            break;

        case hash("--trace-rate", 12):
            check(argc, i + 1);
            opts.trace_rate = Parser::as_stream<int>(argv[++i]);
            break;

        case hash("--driver", 8):
        case hash("--struct", 8):
            check(argc, i + 1);

            if (!strcmp(argv[i + 1], "list")) {
                opts.graph = DiGraph<Node>::Driver::List;
            } else if (!strcmp(argv[i + 1], "matrix")) {
                opts.graph = DiGraph<Node>::Driver::Matrix;
            } else {
                std::cerr << "Invalid graph driver '" << argv[i + 1] << "'\n"
                          << "Valid options are: list, matrix\n";
                exit(EXIT_FAILURE);
            }

            i++;
            break;

        case hash("-r", 2):
        case hash("--route", 7):
        case hash("--routing", 9):
            check(argc, i + 1);

            if (!strcmp(argv[i + 1], "fastest"))
                opts.routing = RouteOpt::Fastest;
            else if (!strcmp(argv[i + 1], "shortest"))
                opts.routing = RouteOpt::Shortest;
            else if (!strcmp(argv[i + 1], "custom"))
                opts.routing = RouteOpt::Custom;
            else {
                std::cerr << "Invalid routing option '" << argv[i + 1] << "'\n"
                          << "Valid options are: fastest, shortest, custom\n";
                exit(EXIT_FAILURE);
            }

            i++;
            break;

        case hash("--cfg", 5):
        case hash("--config", 8):
            check(argc, i + 1);
            opts.coeffs = Parser::as_stream_ptr<Coefficients>(argv[i + 1]);

            i++;
            break;

        case hash("-h", 2):
        case hash("--help", 6):
            std::cout << HELPMSG << "\n";
            exit(EXIT_SUCCESS);
            break;

        default:
            std::cerr << "Unknown option '" << argv[i] << "'\n";
            exit(EXIT_FAILURE);
            return opts;
        }
    }

    return opts;
}

} // namespace cli
#endif // CLI_H
