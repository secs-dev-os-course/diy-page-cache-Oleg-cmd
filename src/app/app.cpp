#include <diy_cache/lab2_api.h>
#include <diy_cache/storage.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <iostream>
#include <unordered_map>

extern std::unordered_map<int, Storage*> g_storages;

void print_hex(const char* buf, size_t size)
{
  for (size_t i = 0; i < size; ++i)
  {
    printf("%02x ", static_cast<unsigned char>(buf[i]));
    if ((i + 1) % 16 == 0)
    {
      printf("\n");
    }
  }
  printf("\n");
}

int main()
{
  std::remove("test_file.dat");

  const char* path = "test_file.dat";
  int fd = lab2_open(path, 10, 4096);
  if (fd < 0)
  {
    std::cerr << "Error opening file: " << fd << std::endl;
    return 1;
  }

  const char* data1 = "This is the first line\n";
  std::cerr << "Writing data1: " << data1;
  ssize_t bytesWritten = lab2_write(fd, data1, strlen(data1));
  if (bytesWritten < 0)
  {
    std::cerr << "Error writing: " << bytesWritten << std::endl;
    lab2_close(fd);
    return 1;
  }

  off_t newOffset = lab2_lseek(fd, 4096, SEEK_SET);
  if (newOffset < 0)
  {
    std::cerr << "lseek error: " << newOffset << std::endl;
    lab2_close(fd);
    return 1;
  }

  const char* data2 = "This is the second line\n";
  std::cerr << "Writing data2: " << data2;
  bytesWritten = lab2_write(fd, data2, strlen(data2));
  if (bytesWritten < 0)
  {
    std::cerr << "Error writing: " << bytesWritten << std::endl;
    lab2_close(fd);
    return 1;
  }

  int fsyncRes = lab2_fsync(fd);
  if (fsyncRes < 0)
  {
    std::cerr << "Error fsync: " << fsyncRes << std::endl;
  }

  struct stat st;
  if (g_storages.find(fd) == g_storages.end())
  {
    std::cerr << "Invalid file descriptor for fstat" << std::endl;
    lab2_close(fd);
    return 1;
  }
  if (fstat(g_storages[fd]->getFd(), &st) == 0)
  {
    std::cout << "File size after writes: " << st.st_size << std::endl;
  }
  else
  {
    std::cerr << "Error getting file size" << std::endl;
    lab2_close(fd);
    return 1;
  }

  lab2_lseek(fd, 0, SEEK_SET);

  off_t fileSize = st.st_size;
  size_t bufferSize = fileSize > 0 ? static_cast<size_t>(fileSize) : 8192;
  char* buf = new char[bufferSize];
  memset(buf, 0, bufferSize);
  ssize_t totalBytesRead = lab2_read(fd, buf, bufferSize);

  if (totalBytesRead < 0)
  {
    std::cerr << "Error reading: " << totalBytesRead << std::endl;
    delete[] buf;
    lab2_close(fd);
    return 1;
  }

  std::cout << "Buffer contents:\n";

  size_t start = 0;
  for (size_t i = 0; i < totalBytesRead; ++i)
  {
    if (buf[i] == '\n')
    {
      std::cout.write(buf + start, i - start + 1);
      start = i + 1;
    }
  }

  if (start < totalBytesRead)
  {
    std::cout.write(buf + start, totalBytesRead - start);
  }

  std::cout << "\nBytes read: " << totalBytesRead << std::endl;

  delete[] buf;
  lab2_close(fd);
  std::remove("test_file.dat");

  return 0;
}