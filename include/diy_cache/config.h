#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>

static constexpr size_t BLOCK_SIZE = 512;

#ifdef DEBUG_CACHE
#define LOG(message) std::cerr << "[Cache] " << message << std::endl
#else
#define LOG(message)
#endif

#endif