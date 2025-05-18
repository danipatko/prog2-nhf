#ifndef UTIL_H
#define UTIL_H

#include <charconv>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <regex>
#include <unordered_set>
#include <vector>

std::string fmt(const int secs);

inline int rand(int min, int max) {
    static std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);

    return distrib(gen);
}

template <typename T> inline size_t true_size(const std::vector<T> &vec) {
    return sizeof(vec) + sizeof(T) * vec.capacity();
}

/**
 * @brief estimate allocated memory of a std::priority_queue
 * @note container type std::vector is assumed.
 */
template <typename T> inline size_t true_size(const std::priority_queue<T> &pq) {
    return sizeof(pq) + sizeof(T) * pq.size() + sizeof(std::vector<T>) + sizeof(T) * pq.c.size();
}

template <typename T> inline size_t true_size(const std::unordered_set<T> &set) {
    return sizeof(set) + sizeof(T) * (set.bucket_count() + set.size());
}

template <typename T> constexpr inline T pow2(T a) {
    return a * a;
}

static const float PI = 3.14159265358979323;
template <typename T> constexpr inline T rad(T deg) {
    return deg * PI / 180.f;
}

struct Hashable {
    virtual size_t hash() const = 0;
};

class Serializable {
    template <class T> static T fopen(const char *filename) {
        T file(filename, T::binary);
        if (!file.is_open()) {
            std::cerr << "failed to open file\n";
            exit(EXIT_FAILURE);
        }

        return file;
    }

  public:
    virtual void write(std::ostream &os) const = 0;
    virtual void read(std::istream &is) = 0;

    template <class T> static void read(const char *filename, T &obj) {
        std::ifstream file = fopen<std::ifstream>(filename);
        obj.read(file);
        file.close();
    }

    template <class T> static void read(const char *filename, std::vector<T> &obj) {
        std::ifstream file = fopen<std::ifstream>(filename);

        size_t count;
        file.read(reinterpret_cast<char *>(&count), sizeof(count));

        obj.resize(count);
        for (int i = 0; i < count; i++)
            obj[i].read(file);

        file.close();
    }

    template <class T> static void read(const char *filename, std::vector<T *> &obj) {
        std::ifstream file = fopen<std::ifstream>(filename);

        size_t count;
        file.read(reinterpret_cast<char *>(&count), sizeof(count));

        obj.resize(count);
        for (int i = 0; i < count; i++) {
            obj[i] = new T();
            obj[i]->read(file);
        }

        file.close();
    }

    template <class T> static void write(const char *filename, const std::vector<T *> &vec) {
        std::ofstream file = fopen<std::ofstream>(filename);

        size_t vec_size = vec.size(); // this is needed for the pointer casting...
        file.write(reinterpret_cast<const char *>(&vec_size), sizeof(vec_size));
        for (auto obj : vec)
            obj->write(file);

        file.close();
    }

    template <class T> static void write(const char *filename, const std::vector<T> &vec) {
        std::ofstream file = fopen<std::ofstream>(filename);

        size_t vec_size = vec.size(); // this is needed for the pointer casting...
        file.write(reinterpret_cast<const char *>(&vec_size), sizeof(vec_size));
        for (auto obj : vec)
            obj.write(file);

        file.close();
    }

    template <class T> static void write(const char *filename, const T &obj) {
        std::ofstream file = fopen<std::ofstream>(filename);
        obj.write(file);
        file.close();
    }
};

namespace Parser {

static std::istringstream ss;

/**
 * Insert operator wrapper for simple strings.
 * T must be a type that has the insert operator overloaded!
 */
template <typename T> T as_stream(const std::string &s) {
    ss = std::istringstream(s.c_str());

    T p;
    ss >> p;

    return p;
}

/**
 * Insert operator wrapper for simple strings.
 * T must be a type that has the insert operator overloaded!
 */
template <typename T> T *as_stream_ptr(const std::string &s) {
    ss = std::istringstream(s.c_str());

    T *p = new T;
    ss >> *p;

    return p;
}

/**
 * Strings do not need to be parsed using the insert operator
 * @returns the input string
 */
template <> inline std::string as_stream(const std::string &s) {
    return s;
}

/**
 * Optimized solution for integers
 * @returns the parsed integer
 */
template <> inline int as_stream(const std::string &s) {
    int result;
    std::from_chars(s.data(), s.data() + s.size(), result);
    return result;
}

/**
 * Optimized solution for floating point numbers
 * @returns the parsed float
 */
template <> inline float as_stream(const std::string &s) {
    return std::atof(s.c_str());
}

/**
 * Insert operator wrapper for simple strings.
 * T must be a type that has the insert operator overloaded!
 */
template <typename T> void as_stream(const std::string &s, T &obj) {
    ss = std::istringstream(s.c_str());
    ss >> obj;
}

/**
 *
 */
inline bool match(const std::string &line, const std::regex &re) {
    return std::regex_search(line, re);
}

/**
 * Extract and parse to given type using a regular expression.
 * @warning T must be a type that has the insert operator overloaded!
 * @param line the input string to search
 * @param re regular expression. Must include a group!
 * @param throw_error throw an exception, if the expression was not matched
 * @param default_value default_value, in case the expression is not matched, but no exception should be thrown
 * @throws a const char* exception, if {throw_error} is true and the expression was not matched
 * @returns the extracted T value
 */
template <typename T> T extract(const std::string &line, const std::regex &re, bool throw_error = false, T default_value = T()) {
    static std::smatch matches;

    if (std::regex_search(line, matches, re) && matches.size() >= 2) {
        return Parser::as_stream<T>(matches[1].str());
    } else {
        if (throw_error)
            throw "No match found!";

        return default_value;
    }
}

template <typename T> void extract(const std::string &line, const std::regex &re, T &buf) {
    static std::smatch matches;

    if (std::regex_search(line, matches, re) && matches.size() >= 2) {
        as_stream<T>(matches[1].str(), buf);
    }
}

}; // namespace Parser

#endif // UTIL_H