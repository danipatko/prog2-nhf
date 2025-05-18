#include "geo.h"
#include "util.h"

#include <cmath>
#include <regex>
#include <vector>

/**
 * Use JS-style regex flavour, and case insensitive.
 */
const std::regex::flag_type flags = std::regex_constants::ECMAScript | std::regex_constants::icase;

const std::regex id_r("\"id\":\"[^\\d]{0,1}(\\d+)\"", flags);
extern const std::regex coordinate_r("\"coordinates\":\\[{0,2}(\\[\\[[\\[\\]\\d\\.,\\-]+\\])", flags);

// const std::regex coordinate_r("\"coordinates\":\\[{2}([\\[\\]\\d\\.\\-,]+)\\]{2}|\"coordinates\":([\\[\\]\\d\\.\\-,]+)", flags);

const std::regex name_r("\"name\":\"([^\"]+)\"", flags);
const std::regex ref_r("\"ref\":\"([^\"]+)\"", flags);

// enums
const std::regex highway_r("\"highway\":\"(motorway|trunk|primary|secondary|tertiary|unclassified|residential|service|living_street|road|motorway_link|trunk_link|primary_link|secondary_link|tertiary_link|pedestrian|footway|cycleway|path|"
                           "bridleway|steps|track|busway|escape|raceway|construction|proposed)\"",
                           flags);

// decimals
const std::regex maxspeed_r("\"maxspeed\":\"(\\d+)\"", flags);
const std::regex lanes_r("\"lanes\":\"(\\d+)\"", flags);

// logical
const std::regex oneway_r("\"oneway\":\"yes\"", flags);
const std::regex bridge_r("\"bridge\":\"yes\"", flags);
const std::regex lit_r("\"toll\":\"yes\"", flags);
const std::regex toll_r("\"lit\":\"yes\"", flags);

// all multipolygon objects are basically an enclosed circle, handle them as roundabouts
const std::regex roundabout_r("\"junction\":\"roundabout\"|\"type\":\"MultiPolygon\"", flags);

constexpr inline float dms2dec(int deg, int min, double sec, char dir) noexcept {
    return (dir == 'S' || dir == 'W' ? -1 : 1) * (deg + min / 60.0 + sec / 3600.0);
}

Point Point::parse(const std::string &line, bool lon_lat) {
    static const std::regex dec_r(R"(^([+-\.\d]+),([+-\.\d]+)$)");
    static const std::regex dms_r(R"((\d+)°(\d+)'([\d.]+)\"?([NS])\s{0,1}(\d+)°(\d+)'([\d.]+)\"?([EW]))");

    std::smatch match;
    if (std::regex_match(line, match, dec_r)) {
        const float first = std::stof(match[1]), second = std::stof(match[2]);
        return lon_lat ? Point{second, first} : Point{first, second};
    }

    if (std::regex_match(line, match, dms_r)) {
        int deg = std::stoi(match[1]), min = std::stoi(match[2]);
        float sec = std::stof(match[3]);
        char dir = match[4].str()[0];

        const float first = dms2dec(deg, min, sec, dir);

        deg = std::stoi(match[5]), min = std::stoi(match[6]);
        sec = std::stof(match[7]);
        dir = match[8].str()[0];

        const float second = dms2dec(deg, min, sec, dir);

        return lon_lat ? Point{second, first} : Point{first, second};
    }

    throw std::invalid_argument("Invalid coordinate format");
}

float Point::distance_sq(const Point &p1, const Point &p2) {
    return pow2(p1.x - p2.x) + pow2(p1.y - p2.y);
}

float Point::distance(const Point &p1, const Point &p2) {
    return std::sqrt(distance_sq(p1, p2));
}

/// source: https://en.wikipedia.org/wiki/Haversine_formula#Formulation
float Point::haversine(const Point &p1, const Point &p2) {
    const float x1 = rad(p1.x), y1 = rad(p1.y);
    const float x2 = rad(p2.x), y2 = rad(p2.y);

    const float h = pow2(std::sin((x2 - x1) / 2)) +    //
                    std::cos(x1) * std::cos(x2) *      //
                        pow2(std::sin((y2 - y1) / 2)); //

    return 2 * EARTH_RADIUS_M * std::asin(std::sqrt(h));
}

bool Point::within(const Point &p1, const Point &p2, const float distance_m) {
    const float h_limit = pow2(std::sin(distance_m / (2 * EARTH_RADIUS_M * 1000)));

    const float x1 = rad(p1.x), y1 = rad(p1.y);
    const float x2 = rad(p2.x), y2 = rad(p2.y);

    const float h = pow2(std::sin((x2 - x1) / 2)) +    //
                    std::cos(x1) * std::cos(x2) *      //
                        pow2(std::sin((y2 - y1) / 2)); //

    return h <= h_limit;
}

void Road::parse(const std::string &line) {
    this->coordinates = Parser::extract<std::vector<Point *>>(line, coordinate_r);

    this->id = Parser::extract<unsigned int>(line, id_r);
    this->ref = Parser::extract<std::string>(line, ref_r);
    this->name = Parser::extract<std::string>(line, name_r);

    this->highway = Parser::extract<HighwayType>(line, highway_r);

    this->maxspeed = Parser::extract<int>(line, maxspeed_r, false, -1);
    this->lanes = Parser::extract<int>(line, lanes_r, false, 1);

    this->roundabout = Parser::match(line, roundabout_r);
    this->lit = Parser::match(line, lit_r);
    this->toll = Parser::match(line, toll_r);
    this->oneway = Parser::match(line, oneway_r);
    this->bridge = Parser::match(line, bridge_r);
}

float Road::visibility() {
    return std::max(maxspeed / 50.f * lanes, 0.5f) * rating();
}

float Road::rating() {
    switch (highway) {
    case HighwayType::motorway:
        return 64;

    case HighwayType::primary:
    case HighwayType::motorway_link:
        return 32;

    case HighwayType::secondary:
    case HighwayType::primary_link:
        return 16;

    case HighwayType::trunk:
    case HighwayType::secondary_link:
        return 8;

    case HighwayType::tertiary:
    case HighwayType::trunk_link:
        return 4;

    // basic roads
    case HighwayType::tertiary_link:
    case HighwayType::unclassified:
    case HighwayType::residential:
    case HighwayType::service:
    case HighwayType::living_street:
    case HighwayType::road:
        return 2;

    // Non-motorized paths
    case HighwayType::pedestrian:
    case HighwayType::footway:
    case HighwayType::cycleway:
    case HighwayType::path:
        return 1;

    default:
        return 0.01;
    }
}

void Road::write(std::ostream &os) const {
    os.write(reinterpret_cast<const char *>(&id), sizeof(id));

    size_t coordinates_size = coordinates.size();
    os.write(reinterpret_cast<const char *>(&coordinates_size), sizeof(coordinates_size));
    for (const Point *pt : coordinates) {
        pt->write(os);
    }

    os.write(reinterpret_cast<const char *>(&highway), sizeof(highway));

    size_t name_size = name.size();
    os.write(reinterpret_cast<const char *>(&name_size), sizeof(name_size));
    os.write(name.c_str(), name_size);

    size_t ref_size = ref.size();
    os.write(reinterpret_cast<const char *>(&ref_size), sizeof(ref_size));
    os.write(ref.c_str(), ref_size);

    os.write(reinterpret_cast<const char *>(&roundabout), sizeof(roundabout));
    os.write(reinterpret_cast<const char *>(&oneway), sizeof(oneway));
    os.write(reinterpret_cast<const char *>(&bridge), sizeof(bridge));
    os.write(reinterpret_cast<const char *>(&maxspeed), sizeof(maxspeed));
    os.write(reinterpret_cast<const char *>(&lanes), sizeof(lanes));
    os.write(reinterpret_cast<const char *>(&toll), sizeof(toll));
    os.write(reinterpret_cast<const char *>(&lit), sizeof(lit));
}

void Road::read(std::istream &is) {
    is.read(reinterpret_cast<char *>(&id), sizeof(id));

    size_t coordinate_size;
    is.read(reinterpret_cast<char *>(&coordinate_size), sizeof(coordinate_size));
    coordinates.resize(coordinate_size, 0);
    for (size_t i = 0; i < coordinate_size; ++i) {
        coordinates[i] = new Point();
        coordinates[i]->read(is);
    }

    is.read(reinterpret_cast<char *>(&highway), sizeof(highway));

    size_t name_size;
    is.read(reinterpret_cast<char *>(&name_size), sizeof(name_size));
    name.resize(name_size);
    is.read(&name[0], name_size);

    size_t ref_size;
    is.read(reinterpret_cast<char *>(&ref_size), sizeof(ref_size));
    ref.resize(ref_size);
    is.read(&ref[0], ref_size);

    is.read(reinterpret_cast<char *>(&roundabout), sizeof(roundabout));
    is.read(reinterpret_cast<char *>(&oneway), sizeof(oneway));
    is.read(reinterpret_cast<char *>(&bridge), sizeof(bridge));
    is.read(reinterpret_cast<char *>(&maxspeed), sizeof(maxspeed));
    is.read(reinterpret_cast<char *>(&lanes), sizeof(lanes));
    is.read(reinterpret_cast<char *>(&toll), sizeof(toll));
    is.read(reinterpret_cast<char *>(&lit), sizeof(lit));
}
