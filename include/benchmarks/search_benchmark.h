#ifndef SEARCH_BENCHMARK_H
#define SEARCH_BENCHMARK_H

#include <chrono>
#include <string>
#include <vector>

std::string GenerateUniqueFilenameS();

void GenerateRandomData(const std::string& filename, std::size_t num_elements,
                        int min_value, int max_value);
bool SearchElementInFileBlock(int fd, int target, std::size_t blockSize);

#endif
