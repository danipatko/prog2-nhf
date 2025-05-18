#ifndef CONFIG_H
#define CONFIG_H

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#define OS_WINDOWS

#include <Windows.h>
#include <cstdio>

//
#elif __APPLE__

throw "UNSUPPORTED";

//
#elif defined(__linux__) || defined(__unix__)

#define OS_LINUX

#endif

// ---------

#include <iostream>

inline void dbg() {
    std::cout << "";
}

#endif
