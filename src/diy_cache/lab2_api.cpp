#include <diy_cache/cache.h>
#include <diy_cache/lab2_api.h>
#include <diy_cache/storage.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <unordered_map>

#ifdef DEBUG_CACHE
#define LOG(message) std::cerr << "[Cache] " << message << std::endl
#else
#define LOG(message)
#endif

static Cache *g_cache = nullptr;
static std::unordered_map<int, Storage *> g_storages;
static std::unordered_map<int, off_t> g_offsets;

int lab2_open(const char *path, size_t cache_size, size_t page_size)
{
  Storage *storage = new Storage(path);

  int result = storage->open(O_RDWR | O_CREAT, 0644);
  if (result < 0)
  {
    delete storage;
    return result;
  }

  if (g_cache == nullptr)
  {
    g_cache = new Cache(PageSize(page_size), CacheSize(cache_size));
  }

  static int next_fd = 3;
  g_storages[next_fd] = storage;
  g_offsets[next_fd] = 0;
  return next_fd++;
}

int lab2_close(int fd)
{
  auto it = g_storages.find(fd);

  if (it == g_storages.end())
  {
    return -1;
  }

  Storage *storage = it->second;
  if (g_cache != nullptr)
  {
    g_cache->close(storage);
  }

  int result = storage->close();
  g_storages.erase(fd);
  g_offsets.erase(fd);
  delete storage;

  if (g_storages.empty() && g_cache != nullptr)
  {
    delete g_cache;
    g_cache = nullptr;
  }
  return result;
}

ssize_t lab2_read(int fd, void *buf, size_t count)
{
  if (g_cache == nullptr) return -1;

  auto itStorage = g_storages.find(fd);
  auto itOffset = g_offsets.find(fd);
  if (itStorage == g_storages.end() || itOffset == g_offsets.end())
  {
    return -1;
  }

  Storage *storage = itStorage->second;
  off_t &offset = itOffset->second;

  size_t totalBytesRead = 0;
  while (count > 0)
  {
    ssize_t bytesRead = g_cache->read(storage, buf, count, offset);

    if (bytesRead < 0)
    {
      return bytesRead;
    }
    if (bytesRead == 0)
    {
      break; // EOF
    }

    totalBytesRead += static_cast<size_t>(bytesRead);
    offset += static_cast<size_t>(bytesRead);
    count -= static_cast<size_t>(bytesRead);
    buf = (char *)buf + bytesRead;
  }

  return static_cast<ssize_t>(totalBytesRead);
}

ssize_t lab2_write(int fd, const void *buf, size_t count)
{
  if (g_cache == nullptr) return -1;

  auto itStorage = g_storages.find(fd);
  auto itOffset = g_offsets.find(fd);
  if (itStorage == g_storages.end() || itOffset == g_offsets.end())
  {
    return -1;
  }

  Storage *storage = itStorage->second;
  off_t &offset = itOffset->second;

  ssize_t totalBytesWritten = 0;
  while (count > 0)
  {
    ssize_t bytesWritten = storage->write(offset, buf, count);

    if (bytesWritten < 0)
    {
      return bytesWritten;
    }

    if (bytesWritten > 0)
    {
      totalBytesWritten += bytesWritten;
      offset += bytesWritten;
      count -= static_cast<size_t>(bytesWritten);
      buf = (char *)buf + bytesWritten;
    }
  }

  return totalBytesWritten;
}

off_t lab2_lseek(int fd, off_t offset, int whence)
{
  auto itStorage = g_storages.find(fd);
  if (itStorage == g_storages.end())
  {
    return -1; // error: file not open
  }

  off_t newOffset;
  switch (whence)
  {
    case SEEK_SET:
      newOffset = offset;
      break;
    case SEEK_CUR:
      newOffset = g_offsets[fd] + offset;
      break;
    case SEEK_END:
      return -1;
    default:
      return -1;
  }

  g_offsets[fd] = newOffset;
  return newOffset;
}

int lab2_fsync(int fd)
{
  if (!g_cache) return -1;

  auto it = g_storages.find(fd);
  if (it == g_storages.end())
  {
    return -1;
  }
  return g_cache->sync(it->second);
}

size_t lab2_get_page_size()
{
  if (g_cache)
  {
    return g_cache->getPageSize();
  }
  else
  {
    return 4096;
  }
}