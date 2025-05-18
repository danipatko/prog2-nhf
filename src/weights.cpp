#include "weights.h"
#include "algorithm.h"
#include "geo.h"

#include <cassert>

const static Coefficients DEFAULT_COEFFS = {
    .slow = 100,
    .time = 1000,
    .distance = 200,
    .turn_penalty = 1000,
    .nonroad_penalty = 10000000,
    .rating = 1000,
    .tolls = 0,
};

/**
 * @brief Calculates the angle of two segments following each other
 * Can be used for turn detection
 */
float angle(const Point &from, const Point &to, const Point &prev) {
    if (from == to || to == prev || from == prev) {
        return 0;
    }

    // we're looking for the angle between a and b
    const float side_a = Point::haversine(prev, from);
    const float side_b = Point::haversine(from, to);
    const float side_c = Point::haversine(prev, to);

    return std::acos((pow2(side_c) - pow2(side_a) - pow2(side_b)) / (-2 * side_a * side_b));
}

/**
 * @brief Filters roads that are not traversable by car
 * @returns true, if the road is not a road for cars
 */
bool is_nonroad(const HighwayType &highway) {
    switch (highway) {
    case HighwayType::pedestrian:
    case HighwayType::footway:
    case HighwayType::cycleway:
    case HighwayType::path:
    case HighwayType::bridleway:
    case HighwayType::steps:

    case HighwayType::track:
    case HighwayType::busway:
    case HighwayType::escape:
    case HighwayType::raceway:

    case HighwayType::construction:
    case HighwayType::proposed:
    case HighwayType::unclassified:
    case HighwayType::service:
    case HighwayType::unknown:
        return true;

    default:
        return false;
    }
}

float Fastest::get(const Node &from, const Node &to, const Node *prev) const {
    static const float limit = M_PI / 3;
    float extra = 0;

    if (prev != nullptr) {
        const float alpha = angle(*from.loc, *to.loc, *prev->loc);
        // penalyze turns!
        if (alpha != 0 && alpha < limit) {
            extra += 15000 * (limit - alpha);
        }
    }

    const float speed_avg = (from.road->maxspeed + to.road->maxspeed) / 2.f;
    const float rating_avg = std::max(1.f, (from.road->rating() + to.road->rating()) / 2.f);

    // metres
    const float s = Point::haversine(from, to);
    // in m/s (base velocity is 30 km/h)
    const float v = std::max(30.f, speed_avg) / 3.6f;

    // reward staying on the same road
    if (from.road->id != to.road->id) {
        extra += 200;
    }

    // roads to avoid
    if (is_nonroad(from.road->highway) && is_nonroad(to.road->highway)) {
        extra += 1000.f;
    }

    // time + inverse rating
    return extra + (s / v) * 500 + 1 / rating_avg * 100;
}

float Custom::get(const Node &from, const Node &to, const Node *prev) const {
    static const float turn_angle_limit = M_PI / 3;

    // in m, m/s, s
    const float distance = Point::haversine(from, to);
    const float speed = std::max(30.f, (from.road->maxspeed + to.road->maxspeed) / 2.f) / 3.6f;
    const float time = distance / speed;

    float total = coeffs.distance * distance + coeffs.slow * speed + coeffs.time * time;

    if (coeffs.rating != 0) {
        // rating is inversed (from high ranking: 1 -> low: 64)
        total += coeffs.rating * 64 / std::max(1.f, (from.road->rating() + to.road->rating()) / 2.f);
    }

    if (coeffs.nonroad_penalty != 0 && (is_nonroad(from.road->highway) && is_nonroad(to.road->highway))) {
        total += coeffs.nonroad_penalty;
    }

    if (coeffs.tolls != 0 && (from.road->toll && to.road->toll)) {
        total += coeffs.tolls;
    }

    if (coeffs.turn_penalty != 0 && prev != nullptr) {
        const float alpha = angle(*from.loc, *to.loc, *prev->loc);

        if (alpha != 0 && alpha < turn_angle_limit) {
            total += coeffs.turn_penalty * (turn_angle_limit - alpha);
        }
    }

    return total;
}

/**
 * @brief Create a Weight instance
 * @param type the routing option to use (Fastest, Shortest, or Custom)
 * @param coeffs custom coefficients to use with the Custom option
 */
Weight<Node> *create(RouteOpt type, const Coefficients *coeffs) {
    switch (type) {
    case RouteOpt::Fastest:
        return new Fastest;
    case RouteOpt::Shortest:
        return new Shortest;
    default:
        return new Custom(coeffs == nullptr ? DEFAULT_COEFFS : *coeffs);
    }
}
