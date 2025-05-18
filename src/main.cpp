#include "algorithm.h"
#include "cli.h"
#include "config.h" // IWYU pragma: keep
#include "diagnostics.h"
#include "lib.h"
#include "network.h"
#include "util.h" // IWYU pragma: keep
#include <iomanip>
#include <ostream>

// #include "memtrace.h" // IWYU pragma: keep

#undef MEMTRACE

Algorithm<Node> *algoselect(const Algorithm<Node>::Driver &driver, const DiGraph<Node> &graph, Weight<Node> *weight) {
    switch (driver) {
    case Algorithm<Node>::Driver::Dijkstra:
        return new Dijkstra<Node>(graph, *weight);

    case Algorithm<Node>::Driver::AStar:
        return new AStar<Node>(graph, *weight, heuristic);

    case Algorithm<Node>::Driver::BFS:
        return new BFS<Node>(graph);

    case Algorithm<Node>::Driver::DFS:
        return new DFS<Node>(graph);

    default:
        throw std::invalid_argument("Invalid algorithm");
    }
}

int closest(const DiGraph<Node> &graph, const Point &target) {
    int min_idx = 0;
    float min_dist = FMAX;

    for (int i = 0; i < graph.size(); i++) {
        const float dist = Point::haversine(target, graph.at(i));
        if (dist < min_dist) {
            min_dist = dist;
            min_idx = i;
        }
    }

    return min_idx;
}

int main(int argc, char *argv[]) {
// support unicode on Windows
#ifdef OS_WINDOWS
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);

    // std::cout << "Árvíztűrő tükörfúrógép\n";
#endif

    cli::Options options = cli::parse(argc, argv);

    Bench load_b("Loading files");
    std::vector<Road *> roads = loader::from_file(options.map);
    load_b.eval(true);

    Bench construct_b("Graph construction");
    DiGraph<Node> graph = loader::construct(roads, options);
    construct_b.eval(true);

    int source, target;

    // by default, choose two random points for source and target
    if (options.source == options.target) {
        source = rand(0, graph.size());
        target = rand(0, graph.size());
    } else {
        source = closest(graph, options.source);
        target = closest(graph, options.target);
    }

    if (source == target) {
        std::cerr << "source cannot be the same as the target!\n";
        return 1;
    }

    // ---

    Weight<Node> *weight = create(options.routing, options.coeffs);
    Algorithm<Node> *algo = algoselect(options.algorithm, graph, weight);

    Bench algo_b("Search algorithm");
    algo->run(source, target, true);
    std::vector<int> path = algo->reconstruct(source, target);
    algo_b.eval(true);

    // ---

    float distance = 0, time = 0;

    // for (int i = 0; i < path.size(); i++)
    //     std::cout << path[i] << " ";
    // std::cout << "\n";

    for (int i = 1; i < path.size(); i++) {
        const Node from = graph.at(path[i - 1]), to = graph.at(path[i]);
        const float s = Point::haversine(from, to);
        const float v = std::max(30.f, (from.road->maxspeed + to.road->maxspeed) / 2.f) / 3.6f;

        distance += s;
        time += s / v;
    }

    std::cout << std::setprecision(7);

    std::cout << "\nAlgorithm Diagnostics" << std::endl
              << "  Memory allocated by graph         " << std::setw(9) << graph.size_of() / pow2(1024.f) << " MB" << std::endl
              << "  Memory allocated by algorithm     " << std::setw(9) << algo->size_of() / pow2(1024.f) << " MB" << std::endl
              << "  Total steps                      " << std::setw(13) << algo->steps << "" << std::endl
              << "  Total comparisons                " << std::setw(13) << algo->comparisons << "" << std::endl
              << "  Total memory operations          " << std::setw(13) << algo->memops << "" << std::endl
              << std::endl;

    std::cout << "Route Information" << std::endl
              << "  Point-to-Point distance  " << std::setw(8) << Point::haversine(graph.at(target), graph.at(source)) / 1000 << " km" << std::endl
              << "  Route distance           " << std::setw(8) << distance / 1000.f << " km" << std::endl
              << "  Estimated time               " << fmt(time) << std::endl
              << std::endl;

    Network network = Network(graph, roads);
    network.run(*algo, path, target, source, options);

    delete algo;
    delete weight;

    return 0;
}
