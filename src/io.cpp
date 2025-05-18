#include "geo.h"
#include "weights.h"

#include <cassert>
#include <iomanip>
#include <iostream>

/**
 * @note The longitude, latitude order is the standard convention for
 * representing geographic coordinates in GeoJSON
 */
std::istream &operator>>(std::istream &is, Point &point) {
  char c;
  double d;

  is >> c;
  assert(c == '[');

  is >> d;
  point.x = d;

  is >> c;
  assert(c == ',');

  is >> d;
  point.y = d;

  is >> c;
  assert(c == ']');

  assert(point.y > -90 && point.x < 90);
  assert(point.y > -180 && point.x < 180);

  return is;
}

std::ostream &operator<<(std::ostream &os, const Point &point) {
  os << '[' << std::setprecision(10) << point.x << ',' << std::setprecision(10)
     << point.y << ']';
  return os;
}

std::istream &operator>>(std::istream &is, std::vector<Point> &points) {
  char c;
  Point p;

  is >> c;
  assert(c == '[');

  while (true) {
    is >> p;
    points.push_back(p);

    is >> c;
    assert(c == ',' || c == ']');

    if (c == ']')
      break;
  }

  return is;
}

std::istream &operator>>(std::istream &is, std::vector<Point *> &points) {
  static char c;

  is >> c;
  assert(c == '[');

  while (true) {
    Point *p = new Point();
    is >> *p;

    points.push_back(p);

    is >> c;
    assert(c == ',' || c == ']');

    if (c == ']')
      break;
  }

  return is;
}

std::ostream &operator<<(std::ostream &os, Point &point) {
  os << '[' << point.x << ',' << point.y << ']';
  return os;
}

std::vector<std::pair<std::string, HighwayType>> const table = {
    {"motorway", HighwayType::motorway},
    {"trunk", HighwayType::trunk},
    {"primary", HighwayType::primary},
    {"secondary", HighwayType::secondary},
    {"tertiary", HighwayType::tertiary},
    {"unclassified", HighwayType::unclassified},
    {"residential", HighwayType::residential},
    {"service", HighwayType::service},
    {"living_street", HighwayType::living_street},
    {"road", HighwayType::road},

    {"motorway_link", HighwayType::motorway_link},
    {"trunk_link", HighwayType::trunk_link},
    {"primary_link", HighwayType::primary_link},
    {"secondary_link", HighwayType::secondary_link},
    {"tertiary_link", HighwayType::tertiary_link},

    {"pedestrian", HighwayType::pedestrian},
    {"footway", HighwayType::footway},
    {"cycleway", HighwayType::cycleway},
    {"path", HighwayType::path},
    {"bridleway", HighwayType::bridleway},
    {"steps", HighwayType::steps},

    {"track", HighwayType::track},
    {"busway", HighwayType::busway},
    {"escape", HighwayType::escape},
    {"raceway", HighwayType::raceway},

    {"construction", HighwayType::construction},
    {"proposed", HighwayType::proposed},
    {"unknown", HighwayType::unknown},
};

HighwayType parse_highway(const std::string &str) {
  for (auto it : table) {
    if (it.first == str)
      return it.second;
  }

  throw "invalid highway type!";
}

std::istream &operator>>(std::istream &is, HighwayType &highway_type) {
  static std::string str;
  is >> str;

  highway_type = parse_highway(str);

  return is;
}

std::ostream &operator<<(std::ostream &os, const HighwayType &highway_type) {
  for (auto it : table) {
    if (it.second == highway_type) {
      os << it.first;
      break;
    }
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const Road &road) {
  os << "(w" << std::setw(7) << road.id << "/" << road.highway << ") "
     << road.getname() << " | ";
  if (road.maxspeed > 0)
    os << "max speed: " << road.maxspeed << " | ";
  if (road.lanes > 1)
    os << road.lanes << " lanes | ";
  if (road.roundabout)
    os << "roundabout | ";
  if (road.bridge)
    os << "bridge | ";
  if (road.oneway)
    os << "oneway | ";
  if (road.roundabout)
    os << "roundabout | ";

  return os;
}

std::ostream &operator<<(std::ostream &os, const BBox &bbox) {
  os << "bbox " << bbox.top_left() << " " << bbox.bottom_right();
  return os;
}

std::istream &operator>>(std::istream &is, Coefficients &coeffs) {
  float f;
  char c;

  is >> coeffs.slow;
  is >> c;
  assert(c == ',' || c == '|');
  is >> coeffs.time;
  is >> c;
  assert(c == ',' || c == '|');
  is >> coeffs.distance;
  is >> c;
  assert(c == ',' || c == '|');
  is >> coeffs.turn_penalty;
  is >> c;
  assert(c == ',' || c == '|');
  is >> coeffs.nonroad_penalty;
  is >> c;
  assert(c == ',' || c == '|');
  is >> coeffs.rating;
  is >> c;
  assert(c == ',' || c == '|');
  is >> coeffs.tolls;

  return is;
}

std::string fmt(const int secs) {
  const int hours = secs / 3600;
  const int minutes = (secs % 3600) / 60;

  std::ostringstream ss;
  ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2)
     << std::setfill('0') << minutes;
  return ss.str();
}
