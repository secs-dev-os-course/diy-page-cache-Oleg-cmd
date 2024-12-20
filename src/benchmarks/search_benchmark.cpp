// search_benchmark.cpp
#include <benchmarks/search_benchmark.h>
#include <diy_cache/lab2_api.h>
#include <sys/stat.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

std::string GenerateUniqueFilenameS()
{
  auto now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::stringstream filename;
  filename << "benchmark_data" << now << "_" << std::this_thread::get_id()
           << ".bin";
  return filename.str();
}

void GenerateRandomData(const std::string& filename, std::size_t num_elements,
                        int min_value, int max_value)
{
  int fd = lab2_open(filename.c_str());
  if (fd == -1)
  {
    std::cerr << "Failed to lab2_open file for writing: " << filename
              << ", error: " << strerror(errno) << std::endl;
    return;
  }
  std::mt19937 gen(std::random_device{}());
  std::uniform_int_distribution<int> dist(min_value, max_value);

  for (std::size_t i = 0; i < num_elements; ++i)
  {
    int num = dist(gen);
    ssize_t written = lab2_write(fd, &num, sizeof(num));
    if (written == -1)
    {
      std::cerr << "Failed to lab2_write to file, error: " << strerror(errno)
                << std::endl;
      lab2_close(fd);
      return;
    }
  }
  lab2_close(fd);
}

bool SearchElementInFileBlock(int fd, int target, std::size_t blockSize)
{
  if (lab2_lseek(fd, 0, SEEK_SET) < 0)
  {
    return false;
  }

  std::vector<int> buffer(blockSize / sizeof(int));
  struct stat st;
  // if (fstat(g_storages[fd]->getFd(), &st) < 0) {
  //     return false;
  // }

  size_t file_size = st.st_size;
  size_t bytes_read = 0;

  while (bytes_read < file_size)
  {
    ssize_t bytesRead = lab2_read(fd, buffer.data(), blockSize);
    if (bytesRead <= 0)
    {
      break;
    }

    std::size_t elementsRead = bytesRead / sizeof(int);
    for (std::size_t i = 0; i < elementsRead; ++i)
    {
      if (buffer[i] == target)
      {
        return true;
      }
    }

    bytes_read += bytesRead;
  }

  return false;
}
