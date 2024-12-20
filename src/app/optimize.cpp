#include <benchmarks/linreg_benchmark.h>
#include <benchmarks/search_benchmark.h>
#include <diy_cache/cache.h>
#include <diy_cache/lab2_api.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <vector>

struct BenchmarkResult
{
  size_t page_size;
  size_t cache_size;
  size_t block_size;
  double time;

  bool operator<(const BenchmarkResult& other) const
  {
    return time < other.time;
  }
};

std::string GenerateUniqueFilename(const std::string& prefix)
{
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream filename;
  filename << prefix << "_" << now << "_" << std::this_thread::get_id()
           << ".txt";
  return filename.str();
}

void TestLinearRegression(int iterations, std::size_t num_points,
                          std::size_t block_size, size_t page_size,
                          size_t cache_size,
                          std::vector<BenchmarkResult>& results)
{
  const std::string filename = GenerateUniqueFilename("linear_regression");
  int fd = lab2_open(filename.c_str(), cache_size, page_size);
  if (fd < 0)
  {
    std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    return;
  }

  GenerateLinearData(fd, num_points);

  double total_time = 0.0;
  for (int i = 0; i < iterations; ++i)
  {
    if (lab2_lseek(fd, 0, SEEK_SET) == -1)
    {
      perror("lab2_lseek failed");
      lab2_close(fd);
    }

    auto start = std::chrono::high_resolution_clock::now();
    LinearRegression(fd, num_points, block_size);
    auto end = std::chrono::high_resolution_clock::now();

    total_time += std::chrono::duration<double>(end - start).count();
  }

  double average_time = total_time / iterations;
  results.push_back({page_size, cache_size, block_size, average_time});
  std::cout << "Page size: " << page_size << ", Cache size: " << cache_size
            << ", Block size: " << block_size << ", Time: " << average_time
            << " seconds\n";

  lab2_close(fd);
  std::remove(filename.c_str());
}

void TestSearchBenchmark(int iterations, std::size_t num_elements,
                         int min_value, int max_value, std::size_t block_size,
                         size_t page_size, size_t cache_size,
                         std::vector<BenchmarkResult>& results)
{
  const std::string filename = GenerateUniqueFilename("search_benchmark");
  GenerateRandomData(filename, num_elements, min_value, max_value);

  int fd = lab2_open(filename.c_str(), cache_size, page_size);
  if (fd < 0)
  {
    std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    return;
  }

  double total_time = 0.0;
  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min_value, max_value);

  for (int i = 0; i < iterations; ++i)
  {
    int target = dist(gen);

    auto start = std::chrono::high_resolution_clock::now();
    SearchElementInFileBlock(fd, target, block_size);
    auto end = std::chrono::high_resolution_clock::now();

    total_time += std::chrono::duration<double>(end - start).count();
  }

  double average_time = total_time / iterations;
  results.push_back({page_size, cache_size, block_size, average_time});
  std::cout << "Page size: " << page_size << ", Cache size: " << cache_size
            << ", Block size: " << block_size << ", Time: " << average_time
            << " seconds\n";

  lab2_close(fd);
  std::remove(filename.c_str());
}

void RunBenchmarks()
{
  std::vector<size_t> page_sizes = {32,   64,   128,  256,  512,
                                    1024, 2048, 4096, 8192, 16384};
  std::vector<size_t> cache_sizes = {2, 5, 10, 20, 50, 100, 200, 500};
  std::vector<size_t> block_sizes = {512, 1024, 2048, 4096, 8192};
  int iterations = 100;
  std::size_t num_points = 1000;
  std::size_t num_elements = 1000;
  int min_value = 0;
  int max_value = 2000;

  for (auto& size : page_sizes)
  {
    size = ((size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
  }
  for (auto& size : block_sizes)
  {
    size = ((size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
  }

  std::vector<BenchmarkResult> linear_regression_results;
  std::vector<BenchmarkResult> search_benchmark_results;

  std::cout << "Starting Linear Regression Tests...\n";
  for (size_t page_size : page_sizes)
  {
    for (size_t cache_size : cache_sizes)
    {
      for (size_t block_size : block_sizes)
      {
        TestLinearRegression(iterations, num_points, block_size, page_size,
                             cache_size, linear_regression_results);
      }
    }
  }

  std::cout << "\nStarting Search Benchmark Tests...\n";
  for (size_t page_size : page_sizes)
  {
    for (size_t cache_size : cache_sizes)
    {
      for (size_t block_size : block_sizes)
      {
        TestSearchBenchmark(iterations, num_elements, min_value, max_value,
                            block_size, page_size, cache_size,
                            search_benchmark_results);
      }
    }
  }

  std::sort(linear_regression_results.begin(), linear_regression_results.end());
  std::cout << "\nTop 5 Linear Regression Results:\n";
  for (size_t i = 0; i < std::min(size_t(5), linear_regression_results.size());
       ++i)
  {
    const auto& result = linear_regression_results[i];
    std::cout << "Page size: " << result.page_size
              << ", Cache size: " << result.cache_size
              << ", Block size: " << result.block_size
              << ", Time: " << result.time << " seconds\n";
  }

  std::sort(search_benchmark_results.begin(), search_benchmark_results.end());
  std::cout << "\nTop 5 Search Benchmark Results:\n";
  for (size_t i = 0; i < std::min(size_t(5), search_benchmark_results.size());
       ++i)
  {
    const auto& result = search_benchmark_results[i];
    std::cout << "Page size: " << result.page_size
              << ", Cache size: " << result.cache_size
              << ", Block size: " << result.block_size
              << ", Time: " << result.time << " seconds\n";
  }
}

int main()
{
  RunBenchmarks();
  return 0;
}
