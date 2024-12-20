#ifndef STORAGE_H
#define STORAGE_H

#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "config.h"

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
  [[nodiscard]] off_t getFileSize() const;

  static bool checkAlignment(const void* buf, off_t offset, size_t count)
  {
    return ((reinterpret_cast<uintptr_t>(buf) & (BLOCK_SIZE - 1)) == 0 &&
            (offset & (BLOCK_SIZE - 1)) == 0 &&
            (count & (BLOCK_SIZE - 1)) == 0);
  }

 private:
  int fd;
  std::string filePath;
  off_t fileSize;
};

#endif