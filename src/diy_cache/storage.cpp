#include <diy_cache/storage.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
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
        if (errno == EINVAL) {
            flags &= ~O_DIRECT;
            fd = ::open(filePath.c_str(), flags, mode);
            if (fd == -1) {
                std::cerr << "Error opening file without O_DIRECT: " << strerror(errno) << std::endl;
                return -errno;
            }
        } else {
            std::cerr << "Error opening file: " << strerror(errno) << std::endl;
            return -errno;
        }
    }

    struct stat st;
    if (fstat(fd, &st) == 0)
    {
        fileSize = st.st_size;
    }
    else
    {
        std::cerr << "Error getting file size: " << strerror(errno) << std::endl;
        fileSize = 0;
    }

    if ((flags & O_CREAT) && fileSize == 0) {
        if (fallocate(fd, 0, 0, BLOCK_SIZE) != 0) {
            std::cerr << "Warning: fallocate failed: " << strerror(errno) << std::endl;
        }
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

ssize_t Storage::read(off_t offset, void* buf, size_t count) {
    if ((reinterpret_cast<uintptr_t>(buf) & (BLOCK_SIZE - 1)) != 0 ||
        (offset & (BLOCK_SIZE - 1)) != 0 ||
        (count & (BLOCK_SIZE - 1)) != 0) {
        std::cerr << "Alignment error for O_DIRECT" << std::endl;
        return -EINVAL;
    }
    
    ssize_t bytesRead = pread(fd, buf, count, offset);
    if (bytesRead == ssize_t(-1)) {
        std::cerr << "Error reading file: " << strerror(errno) << " at offset "
                  << offset << std::endl;
        return -errno;
    }
    return bytesRead;
}


ssize_t Storage::write(off_t offset, const void* buf, size_t count) {
    if ((reinterpret_cast<uintptr_t>(buf) & (BLOCK_SIZE - 1)) != 0 ||
        (offset & (BLOCK_SIZE - 1)) != 0 ||
        (count & (BLOCK_SIZE - 1)) != 0) {
        std::cerr << "Alignment error for O_DIRECT" << std::endl;
        return -EINVAL;
    }

    ssize_t bytesWritten = pwrite(fd, buf, count, offset);
    if (bytesWritten == -1) {
        std::cerr << "Error writing file: " << strerror(errno) << " at offset "
                  << offset << std::endl;
        return -errno;
    }
      
    fileSize = std::max((off_t)(offset + count), fileSize);
    return bytesWritten;
}


off_t Storage::getFileSize() const{
    return fileSize;
}