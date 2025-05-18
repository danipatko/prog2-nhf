#ifndef GEO_H
#define GEO_H

#include "consts.h"
#include "util.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <ostream>
#include <string>
#include <vector>

/**
 * Earth's average meridional radius
 */
static const float EARTH_RADIUS_M = 6367449.f;

/**
 * Simple geospatial point
 */
struct Point : public Hashable, public Serializable {
    /**
     * longitude, must be between -180..180
     */
    float x;

    /**
     * latitude, must be between -90..90
     */
    float y;

    /**
     * Constructor
     */
    Point(float lon = 0.f, float lat = 0.f) : x(lat), y(lon) {};

    /**
     * Hashing impl
     */
    size_t hash() const {
        static const double precision = 1e8;

        const std::size_t h1 = std::hash<int64_t>{}(std::round(x * precision));
        const std::size_t h2 = std::hash<int64_t>{}(std::round(y * precision));

        return h1 ^ (h2 << 1);
    }

    /**
     * Serialize impl
     */
    void write(std::ostream &os) const {
        os.write(reinterpret_cast<const char *>(&x), sizeof(x));
        os.write(reinterpret_cast<const char *>(&y), sizeof(y));
    }

    /**
     * Deserialize impl
     */
    void read(std::istream &is) {
        is.read(reinterpret_cast<char *>(&x), sizeof(x));
        is.read(reinterpret_cast<char *>(&y), sizeof(y));
    }

    bool operator==(const Point &rhs) const {
        return x == rhs.x && y == rhs.y;
    }

    bool operator!=(const Point &rhs) const {
        return x != rhs.x || y != rhs.y;
    }

    /**
     * @brief *Squared* distance of two points
     * @note The second point defaults to the origin (0,0)
     */
    static float distance_sq(const Point &p1, const Point &p2 = Point());

    /**
     * @brief Distance of two points
     * @note The second point defaults to the origin (0,0)
     */
    static float distance(const Point &p1, const Point &p2 = Point());

    /**
     * @brief Real-world distance of two points using the Haversine-formula, in metres
     */
    static float haversine(const Point &p1, const Point &p2 = Point());

    /**
     * @brief Checks if two points on the map are within a specified distance of each other
     */
    static bool within(const Point &p1, const Point &p2, const float distance_h);

    /**
     * @brief Parses a point, from either of the following formats:
     * simple: "19.05275536,47.46348953"
     * or dms: "47°27'00.0"N19°10'49.0"E"
     * @param line the thing to parse
     * @param lon_lat use longitude,latitude order
     * @warning GeoJSON uses the lon,lat order, however coordinates copied from eg. google maps use lat,lon
     */
    static Point parse(const std::string &line, bool lon_lat = false);
};

/**
 * @brief Bounding box structure
 */
struct BBox {
    /**
     * @brief Top-left geo coordinate of the bounding box
     */
    float x, y;

    /**
     * @brief Bottom-right geo coordinate of the bounding box
     */
    float w, h;

    BBox(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {};

    BBox(const Point &p0, float w, float h) : x(p0.y), y(p0.x), w(w), h(h) {};

    BBox(const Point &p0, const Point &p1) : x(p0.y), y(p0.x), w(p1.y - p0.y), h(p1.x - p0.x) {};

    Point center() const {
        return Point(x + w / 2, y + h / 2);
    }

    Point top_left() const {
        return Point(x, y);
    }

    Point bottom_right() const {
        return Point(x + w, y + h);
    }

    Point top_right() const {
        return Point(x + w, y);
    }

    Point bottom_left() const {
        return Point(x, y + h);
    }

    /**
     * @brief Check if point is within bounds of this
     */
    bool contains(const Point &p) const {
        return contains(p.y, p.x);
    }

    /**
     * @brief Check if point is within bounds of this
     */
    bool contains(float _x, float _y) const {
        return (x <= _x && _x <= x + w) && (y <= _y && _y <= y + h);
    }

    /**
     * @brief Approximation of surface area, defined by this bounding box
     */
    float area() const {
        return pow2(EARTH_RADIUS_M) *                              //
               std::abs(std::sin(rad(y + h)) - std::sin(rad(y))) * //
               std::abs(rad(x + w) - rad(x));
    }

    /**
     * @brief Adjust bounds, so that a specified point falls within the bounds
     */
    void include(const Point &p) {
        include(p.y, p.x);
    }

    /**
     * @brief Adjust bounds, so that a specified point falls within the bounds
     */
    void include(float lon, float lat) {
        if (lon < x)
            x = lon;
        if (lon > x + w)
            w = lon - x;
        if (lat < y)
            y = lat;
        if (lat > y + h)
            h = lat - y;
    }

    static BBox bounds(const std::vector<Point> &points) {
        static const float MAX = std::numeric_limits<float>::max(), LOWEST = std::numeric_limits<float>::lowest();
        BBox bbox(MAX, MAX, LOWEST, LOWEST);

        for (const Point &p : points)
            bbox.include(p);

        return bbox;
    }

    static BBox max() {
        return BBox(FMAX, FMAX, FLOWEST, FLOWEST);
    }
};

// OSM properties

enum class HighwayType : int {
    unknown = 0,

    // Major roads
    motorway,
    trunk,
    primary,
    secondary,
    tertiary,
    unclassified,
    residential,
    service,
    living_street,
    road,

    // Link roads
    motorway_link,
    trunk_link,
    primary_link,
    secondary_link,
    tertiary_link,

    // Non-motorized paths
    pedestrian,
    footway,
    cycleway,
    path,
    bridleway,
    steps,

    // Other
    track,
    busway,
    escape,
    raceway,

    // Construction and planning
    construction,
    proposed,
};

/**
 * OSM properties of a road
 * For more information, see: https://wiki.openstreetmap.org/wiki/Key:<insert-member-name>
 */
struct Road : public Serializable {
    unsigned int id;

    std::vector<Point *> coordinates;

    std::string getname() const {
        if (name.empty() && ref.empty())
            return "unknown road";

        if (name.empty())
            return ref;

        return name;
    }

    /**
     * Type of road, unclassified by default
     */
    HighwayType highway;

    /**
     * Road name, if any
     */
    std::string name;

    /**
     * Road reference name (eg. a number or M0, M31 etc.)
     */
    std::string ref;

    /**
     * Is the road a roundabout?
     * Coodinate array represents a polygon, first and last element should be connected
     */
    bool roundabout;

    /**
     * Road is oneway, the coordinate array determines the direction of the edges.
     */
    bool oneway;

    /**
     * Road is an overpass, should not be connected with intersecting points.
     */
    bool bridge;

    // weighing properties:

    /**
     * Maximal speed
     * -1 by default -> no/unknown speed limit
     */
    int maxspeed;

    /**
     * Number of lanes
     * 1 by default
     */
    int lanes;

    /**
     * Extra routing option, if avoid toll roads is on, the returned weight should be infty
     * false by default
     */
    bool toll;

    /**
     * Road has lighting, may come useful for design
     */
    bool lit;

    /**
     * @brief Preferability of this road.
     */
    float rating();

    /**
     * @brief Zoom visibility is calculated based on this value. Uses `rating()`
     */
    float visibility();

    void write(std::ostream &os) const override;

    void read(std::istream &is) override;

    /**
     * Converts a line of geojson to a road object
     */
    void parse(const std::string &line);

    /**
     * Constructor
     */
    Road() {}

    /**
     * Copy constructor
     * Creates deep copy of all pointer types as well
     */
    Road(const Road &other)
        : id(other.id), highway(other.highway), name(other.name), ref(other.ref), roundabout(other.roundabout), oneway(other.oneway), bridge(other.bridge), maxspeed(other.maxspeed), lanes(other.lanes), toll(other.toll), lit(other.lit) {
        coordinates.reserve(other.coordinates.size());
        for (const auto *pt : other.coordinates)
            coordinates.push_back(new Point(*pt)); // Deep copy
    }

    ~Road() {
        for (Point *p : coordinates)
            delete p;
    }
};

/**
 * @brief Data point for the graph's vertex
 * One-to-one reference to the point
 * One-to-many reference to the road the point belongs to
 */
struct Node {
    Road *road;
    Point *loc;

    Node(Road *road, Point *loc) : road(road), loc(loc) {};

    operator Point &() const {
        return *loc;
    }

    operator Road &() const {
        return *road;
    }
};

// IO

/**
 * Write Point object to stream
 * Format: [%f, %f]
 */
std::ostream &operator<<(std::ostream &os, const Point &point);

/**
 * Parse point from a stream
 * Format: [%f, %f]
 * @throws an assertion error, if the point coordinates are out of range or the format is not correct.
 */
std::istream &operator>>(std::istream &is, Point &point);
std::istream &operator>>(std::istream &is, std::vector<Point> &points);
std::istream &operator>>(std::istream &is, std::vector<Point *> &points);

std::ostream &operator<<(std::ostream &os, const HighwayType &highway_type);
std::istream &operator>>(std::istream &is, HighwayType &highway_type);

std::ostream &operator<<(std::ostream &os, const Road &road);
std::ostream &operator<<(std::ostream &os, const BBox &bbox);

#endif