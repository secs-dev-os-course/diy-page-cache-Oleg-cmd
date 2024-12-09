#include <diy_cache/storage.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

Storage::Storage(const std::string& filePath) : fd(-1), filePath(filePath) {}

Storage::~Storage()
{
  if (fd != -1)
  {
    close();
  }
}

int Storage::open(int flags, mode_t mode)
{
  fd = ::open(filePath.c_str(), flags, mode);
  if (fd == -1)
  {
    std::cerr << "Error opening file: " << strerror(errno) << std::endl;
    return -errno;
  }
  return 0;
}

int Storage::close()
{
  if (::close(fd) == -1)
  {
    std::cerr << "Error closing file: " << strerror(errno) << std::endl;
    return -errno;
  }
  fd = -1;
  return 0;
}

ssize_t Storage::read(off_t offset, void* buf, size_t count)
{
  ssize_t bytesRead = pread(fd, buf, count, offset);
  if (bytesRead == ssize_t(-1))
  {
    std::cerr << "Error reading file: " << strerror(errno) << " at offset "
              << offset << std::endl;
    return -errno;
  }
  return bytesRead;
}

ssize_t Storage::write(off_t offset, const void* buf, size_t count)
{
  ssize_t bytesWritten = pwrite(fd, buf, count, offset);
  if (bytesWritten == -1)
  {
    std::cerr << "Error writing file: " << strerror(errno) << " at offset "
              << offset << std::endl;
    return -errno;
  }

  return bytesWritten;
}