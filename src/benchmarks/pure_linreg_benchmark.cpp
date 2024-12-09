#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
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
  filename << "linear_data_" << now << "_" << std::this_thread::get_id()
           << ".txt";
  return filename.str();
}

void GenerateLinearData(const std::string& filename, std::size_t num_points)
{
  std::ofstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Error opening file for data generation.\n";
    return; // or throw an exception
  }

  std::mt19937 gen(std::random_device{}());
  std::uniform_real_distribution<double> dist(0.0, 10.0);

  double slope = 2.5;
  double intercept = 5.0;

  for (std::size_t i = 0; i < num_points; ++i)
  {
    double x = dist(gen);
    double noise = dist(gen) - 5.0;
    double y = slope * x + intercept + noise;
    file << x << " " << y << "\n";
  }
}

void LinearRegression(const std::string& filename, std::size_t num_points)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Error opening file for linear regression.\n";
    return;
  }

  std::vector<double> x_vals(num_points);
  std::vector<double> y_vals(num_points);

  constexpr size_t buffer_size = 4096;
  char buffer[buffer_size];
  file.rdbuf()->pubsetbuf(buffer, buffer_size);

  for (std::size_t i = 0; i < num_points; ++i)
  {
    file >> x_vals[i] >> y_vals[i];
  }

  double sum_x = 0.0, sum_y = 0.0;
  for (std::size_t i = 0; i < num_points; ++i)
  {
    sum_x += x_vals[i];
    sum_y += y_vals[i];
  }
  double mean_x = sum_x / num_points;
  double mean_y = sum_y / num_points;

  double num = 0.0, den = 0.0;
  for (std::size_t i = 0; i < num_points; ++i)
  {
    num += (x_vals[i] - mean_x) * (y_vals[i] - mean_y);
    den += (x_vals[i] - mean_x) * (x_vals[i] - mean_x);
  }

  double beta_1 = num / den;
  double beta_0 = mean_y - beta_1 * mean_x;
}

int main(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0]
              << " <number_of_iterations> <num_points>\n";
    return 1;
  }

  int iterations = std::stoi(argv[1]);
  std::size_t num_points = std::stoull(argv[2]);

  if (iterations <= 0 || num_points <= 0)
  {
    std::cerr
        << "Invalid arguments: ensure iterations > 0 and num_points > 0.\n";
    return 1;
  }

  const std::string TEMP_FILENAME = GenerateUniqueFilename();
  GenerateLinearData(TEMP_FILENAME, num_points);

  double total_time = 0.0;
  for (int i = 0; i < iterations; ++i)
  {
    auto start = std::chrono::high_resolution_clock::now();
    LinearRegression(TEMP_FILENAME, num_points);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    total_time += elapsed.count();
  }

  std::cout << "Benchmark complete.\n";
  std::cout << std::fixed << std::setprecision(7)
            << "Average LinearRegression execution time: "
            << (total_time / iterations) << " seconds.\n";

  std::remove(TEMP_FILENAME.c_str());
  return 0;
}