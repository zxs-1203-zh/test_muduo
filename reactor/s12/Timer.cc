#include "Timer.h"
#include <atomic>

using namespace muduo;

std::atomic_int64_t Timer::s_numCreated(1);
