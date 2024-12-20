#include <benchmarks/linreg_benchmark.h>
#include <diy_cache/lab2_api.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    std::cerr << "[USAGE] " << argv[0]
              << " <number_of_iterations> <num_points> <block_size>\n";
    return 1;
  }

  int iterations = std::stoi(argv[1]);
  std::size_t num_points = std::stoull(argv[2]);
  std::size_t block_size = std::stoull(argv[3]);

  if (iterations <= 0 || num_points <= 0 || block_size <= 0)
  {
    std::cerr << "[ERROR] Invalid arguments: ensure iterations > 0, num_points "
                 "> 0, and block_size > 0.\n";
    return 1;
  }

  const std::string TEMP_FILENAME = GenerateUniqueFilenameL();
  std::cout << "[INFO] Temporary file: " << TEMP_FILENAME << std::endl;

  int fd = lab2_open(TEMP_FILENAME.c_str());
  if (fd < 0)
  {
    std::cerr << "[ERROR] Failed to open file: " << strerror(errno)
              << std::endl;
    return 1;
  }
  std::cout << "[INFO] File opened successfully." << std::endl;

  std::cout << "[INFO] Generating linear data with " << num_points << " points."
            << std::endl;
  GenerateLinearData(fd, num_points);
  std::cout << "[INFO] Data generation complete." << std::endl;

  double total_time = 0.0;
  std::cout << "[INFO] Starting benchmark with " << iterations << " iterations."
            << std::endl;

  for (int i = 0; i < iterations; ++i)
  {
    if (lab2_lseek(fd, 0, SEEK_SET) == -1)
    {
      perror("[ERROR] lab2_lseek failed");
      lab2_close(fd);
      return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    LinearRegression(fd, num_points, block_size);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();

    // std::cout << "[ITERATION " << (i + 1) << "] Execution time: " <<
    // std::fixed
    //           << std::setprecision(7) << elapsed.count() << " seconds."
    //           << std::endl;
  }

  std::cout << "[BENCHMARK COMPLETE] Average execution time: " << std::fixed
            << std::setprecision(7) << (total_time / iterations)
            << " seconds.\n";

  lab2_close(fd);
  std::remove(TEMP_FILENAME.c_str());
  std::cout << "[INFO] Temporary file removed." << std::endl;

  return 0;
}
