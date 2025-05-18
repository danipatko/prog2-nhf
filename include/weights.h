#ifndef WEIGHTS_H
#define WEIGHTS_H

#include "algorithm.h"
#include "geo.h"

#include <cassert>

enum class RouteOpt {
    Shortest,
    Fastest,
    Custom,
};

struct Coefficients {
    /**
     * @brief inverse road speed multiplier. can be used to penalyze slow roads
     * @note speed is m/s
     */
    float slow;

    /**
     * @brief time multiplier (in seconds)
     */
    float time;

    /**
     * @brief multiplier for distance of the two points (in metres)
     */
    float distance;

    /**
     * @brief multiplier for turn angles (angle is between 0 and PI/3 radians (120deg), sharper turns = more penalty)
     * @note this is very much recommended to prevent nonsense shortcuts
     */
    float turn_penalty;

    /**
     * @brief penalty for roads that are not for cars
     */
    float nonroad_penalty;

    /**
     * @brief multiplier for road ratings. base roads have a penalty of 64, interstates 1 (scaled exponentially)
     */
    float rating;

    /**
     * @brief penalty for toll roads.
     */
    float tolls;
};

/**
 * @brief Parse Coefficents
 * format: 6 numbers separated by ',' or '|'
 * @throws assertion error if the format is incorrect
 */
std::istream &operator>>(std::istream &is, Coefficients &coeffs);

/**
 * @brief Weight that goes for the shortest path
 */
struct Shortest : Weight<Node> {
    /**
     * @brief Weight is the distance in metres + 0.1
     */
    float get(const Node &from, const Node &to, const Node *prev) const override {
        return 0.1f + Point::haversine(from, to);
    }
};

/**
 * @brief Heuristic weight function - the goal is to give an estimate of the distance (weight) without using much compute
 */
struct Heuristic : Weight<Node> {
    float get(const Node &from, const Node &to, const Node *prev) const override {
        return 1.f + 1000 * Point::distance_sq(from, to);
    }
} static const heuristic;

/**
 * @brief Finds the fastest, most sane route.
 */
struct Fastest : Weight<Node> {
    float get(const Node &from, const Node &to, const Node *prev) const override;
};

/**
 * @brief User-adjustable weight class
 */
class Custom : public Weight<Node> {
    const Coefficients coeffs;

  public:
    /**
     * @brief Custom weight construcor
     * @param coeffs the weighing coefficents
     */
    Custom(const Coefficients &coeffs) : coeffs(std::move(coeffs)) {};

    float get(const Node &from, const Node &to, const Node *prev) const override;
};

/**
 * @brief Create a Weight instance
 * @param type the routing option to use (Fastest, Shortest, or Custom)
 * @param coeffs custom coefficients to use with the Custom option
 */
Weight<Node> *create(RouteOpt type = RouteOpt::Fastest, const Coefficients *coeffs = nullptr);

#endif // WEIGHTS_H