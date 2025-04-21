#ifndef UTIL_HPP
#define UTIL_HPP
// #pragma GCC optimize("O2")
#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>
#include <set>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <stdexcept>
#include <numeric>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "request.hpp"




#define DEBUG 1 // 0: Release, 1: Debug
#define PLATFORM 0 // 0: Windows, 1: Linux
enum ACTION{JUMP = 1, PASS = 2, READ = 3};
const int POINT_NUM = 2;
const int REP_NUM = 3;
const int FRE_PER_SLICING = 1800;
const int EXTRA_TIME = 105; // 额外时间片
const int PERCENTAGE = 1; // 磁盘使用率

const int IGNO_TIME = 1;
const int TILE_SIZE = 160;
const int SKIP_TAGS = 0; // 跳过时间片 
const double CUT_OFF = 0.3; // 磁盘使用率的截止值
const int TIME_WINDOW = 800; // 预测时间窗口大小
const int SCH_CONST_1 = 50;
const double WINDOW_PUNISH_1 = 6; // 窗口惩罚系数s
const int SCH_CONST_2 = 50;
const double WINDOW_PUNISH_2 = 6; // 窗口惩罚系数







#if PLATFORM == 1
#define WINDOWS
#include "include/mingw.thread.h"
#include "include/mingw.condition_variable.h"
#include "include/mingw.mutex.h"
#include "include/mingw.future.h"
#include <atomic>
#else
#define LINUX
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>
#include <atomic>
#endif

#if DEBUG == 1
#define DEBUG_PRINT(x) std::cerr << x << std::endl;

#define ASSERT_DEBUG(cond, msg, ...) \
    if (!(cond)) { \
        std::cerr << "Assertion failed: " << (msg) << "\n"; \
        std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << "\n"; \
        std::cerr << "Variables: "; \
        debug_output(__VA_ARGS__); \
        std::cerr << std::endl; \
        assert(cond); \
    }

template <typename T>
void debug_output(const T &arg) {
    std::cerr << arg << " ";
}

template <typename T, typename... Args>
void debug_output(const T &first, const Args &...rest) {
    std::cerr << first << " ";
    debug_output(rest...);
}
#endif



#endif // UTIL_HPP