#pragma once

#include <iostream>

#define DEBUG false

#define assertm(exp, msg) assert(((void)msg, exp))
#define LOG_INFO(msg) std::cout << msg << std::endl
#define LOG_ERROR(msg) std::cerr << msg << std::endl
#define LOG_DEBUG(msg) if (DEBUG) std::cout << msg << std::endl

// #define uint unsigned int
