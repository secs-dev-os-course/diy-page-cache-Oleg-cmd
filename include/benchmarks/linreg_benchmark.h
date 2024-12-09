#ifndef LINREG_BENCHMARK_H
#define LINREG_BENCHMARK_H

#include <cstddef>
#include <string>

std::string GenerateUniqueFilenameL();

void GenerateLinearData(int fd, std::size_t num_points);
void LinearRegression(int fd, std::size_t num_points, std::size_t block_size);

#endif
