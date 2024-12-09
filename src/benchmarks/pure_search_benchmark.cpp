#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

std::string GenerateUniqueFilename()
{
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream filename;
  filename << "benchmark_data" << now << "_" << std::this_thread::get_id()
           << ".bin";
  return filename.str();
}

void GenerateRandomData(const std::string &filename, std::size_t num_elements,
                        int min_value, int max_value)
{
  std::ofstream file(filename, std::ios::binary);
  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min_value, max_value);

  for (std::size_t i = 0; i < num_elements; ++i)
  {
    int num = dist(gen);
    file.write(reinterpret_cast<char *>(&num), sizeof(num));
  }
}

bool SearchElementInFile(const std::string &filename, int target)
{
  std::ifstream file(filename, std::ios::binary);
  int num = 0;
  while (file.read(reinterpret_cast<char *>(&num), sizeof(num)))
  {
    if (num == target)
    {
      return true;
    }
  }
  return false;
}

int main(int argc, char *argv[])
{
  if (argc != 5)
  {
    std::cerr << "Usage: " << argv[0]
              << " <number_of_iterations> <number_of_elements> <min_value> "
                 "<max_value>\n";
    return 1;
  }

  int iterations = std::stoi(argv[1]);
  std::size_t num_elements = std::stoull(argv[2]);
  int min_value = std::stoi(argv[3]);
  int max_value = std::stoi(argv[4]);

  if (iterations <= 0 || num_elements <= 0 || min_value > max_value)
  {
    std::cerr << "Invalid arguments: ensure iterations > 0, num_elements > 0, "
                 "and min_value <= max_value.\n";
    return 1;
  }

  const std::string TEMP_FILENAME = GenerateUniqueFilename();

  std::cout << "Generating random data...\n";
  GenerateRandomData(TEMP_FILENAME, num_elements, min_value, max_value);
  std::cout << "Data generation complete. Number of elements in file: "
            << num_elements << "\n";

  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min_value, max_value);

  double total_time = 0.0;
  for (int i = 0; i < iterations; ++i)
  {
    int target = dist(gen);

    auto start = std::chrono::high_resolution_clock::now();
    bool found = SearchElementInFile(TEMP_FILENAME, target);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();
  }

  std::cout << "Benchmark complete.\n";
  std::cout << "Average search time: " << (total_time / iterations)
            << " seconds.\n";

  std::remove(TEMP_FILENAME.c_str());
  return 0;
}