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

std::string GenerateUniqueFilenameL()
{
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream filename;
  filename << "linear_data_" << now << "_" << std::this_thread::get_id()
           << ".txt";
  return filename.str();
}

void GenerateLinearData(int fd, std::size_t num_points)
{
  std::mt19937 gen(std::random_device{}());
  std::uniform_real_distribution<double> dist(0.0, 10.0);

  double slope = 2.5;
  double intercept = 5.0;

  std::stringstream dataStream;

  for (std::size_t i = 0; i < num_points; ++i)
  {
    double x = dist(gen);
    double noise = dist(gen) - 5.0;
    double y = slope * x + intercept + noise;
    dataStream << std::fixed << std::setprecision(10) << x << " " << y << "\n";
  }

  std::string data = dataStream.str();

  if (lab2_lseek(fd, 0, SEEK_SET) == -1)
  {
    perror("lab2_lseek failed");
    return;
  }

  ssize_t written = lab2_write(fd, data.c_str(), data.length());
  if (written < 0)
  {
    perror("Failed to lab2_write to file");
    return;
  }
}

void LinearRegression(int fd, std::size_t num_points, std::size_t block_size)
{
  std::vector<double> x_vals(num_points);
  std::vector<double> y_vals(num_points);

  char* buf = new char[block_size];
  size_t bytesRemaining = num_points * 2 * sizeof(double);
  size_t i = 0;

  while (bytesRemaining > 0)
  {
    ssize_t bytesRead =
        lab2_read(fd, buf, std::min(block_size, bytesRemaining));
    if (bytesRead < 0)
    {
      perror("lab2_read failed");
      delete[] buf;
      return;
    }

    bytesRemaining -= static_cast<size_t>(bytesRead);

    char* current_ptr = buf;
    while (bytesRead >= static_cast<ssize_t>(2 * sizeof(double)) &&
           i < num_points)
    {
      x_vals[i] = *reinterpret_cast<double*>(current_ptr);
      current_ptr += sizeof(double);
      y_vals[i] = *reinterpret_cast<double*>(current_ptr);
      current_ptr += sizeof(double);
      bytesRead -= 2 * sizeof(double);
      ++i;
    }
  }
  delete[] buf;

  double sum_x = 0.0, sum_y = 0.0;
  for (size_t j = 0; j < num_points; ++j)
  {
    sum_x += x_vals[j];
    sum_y += y_vals[j];
  }

  double mean_x = static_cast<double>(sum_x) / static_cast<double>(num_points);
  double mean_y = static_cast<double>(sum_y) / static_cast<double>(num_points);

  double num = 0.0, den = 0.0;
  for (size_t j = 0; j < num_points; ++j)
  {
    num += (x_vals[j] - mean_x) * (y_vals[j] - mean_y);
    den += (x_vals[j] - mean_x) * (x_vals[j] - mean_x);
  }

  double beta_1 = num / den;
  double beta_0 = mean_y - beta_1 * mean_x;
}
