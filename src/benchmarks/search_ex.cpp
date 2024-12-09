// main.cpp
#include <benchmarks/search_benchmark.h>
#include <diy_cache/lab2_api.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

int main(int argc, char* argv[])
{
  if (argc != 6)
  {
    std::cerr << "Usage: " << argv[0]
              << " <number_of_iterations> <number_of_elements> <min_value> "
                 "<max_value> <block_size>\n";
    return 1;
  }

  int iterations = std::stoi(argv[1]);
  std::size_t num_elements = std::stoull(argv[2]);
  int min_value = std::stoi(argv[3]);
  int max_value = std::stoi(argv[4]);
  std::size_t blockSize = std::stoull(argv[5]);

  if (iterations <= 0 || num_elements <= 0 || min_value > max_value ||
      blockSize <= 0)
  {
    std::cerr << "Invalid arguments: ensure iterations > 0, num_elements > 0, "
                 "min_value <= max_value, and block_size > 0.\n";
    return 1;
  }

  const std::string TEMP_FILENAME = GenerateUniqueFilenameS();

  std::cout << "Generating random data...\n";
  GenerateRandomData(TEMP_FILENAME, num_elements, min_value, max_value);
  std::cout << "Data generation complete. Number of elements in file: "
            << num_elements << "\n";

  int fd = lab2_open(TEMP_FILENAME.c_str());
  if (fd < 0)
  {
    std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    return 1;
  }

  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min_value, max_value);

  double total_time = 0.0;
  for (int i = 0; i < iterations; ++i)
  {
    int target = dist(gen);

    auto start = std::chrono::high_resolution_clock::now();
    bool found = SearchElementInFileBlock(fd, target, blockSize);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();
  }

  std::cout << "Benchmark complete.\n";
  std::cout << "Average search time: " << (total_time / iterations)
            << " seconds.\n";

  lab2_close(fd);
  std::remove(TEMP_FILENAME.c_str());

  return 0;
}
