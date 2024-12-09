#ifndef STORAGE_H
#define STORAGE_H

#include <sys/types.h>
#include <unistd.h>

#include <string>

class Storage
{
 public:
  Storage(const std::string& filePath);
  ~Storage();

  auto read(off_t offset, void* buf, size_t count) -> ssize_t;
  auto write(off_t offset, const void* buf, size_t count) -> ssize_t;
  auto open(int flags, mode_t mode = 0644) -> int;
  auto close() -> int;
  [[nodiscard]] auto getFd() const -> int { return fd; }

 private:
  int fd;
  std::string filePath;
};

#endif