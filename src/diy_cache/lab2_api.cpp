#include <diy_cache/cache.h>
#include <diy_cache/lab2_api.h>
#include <diy_cache/storage.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

static Cache* g_cache = nullptr;
std::unordered_map<int, Storage*> g_storages;
static std::unordered_map<int, off_t> g_offsets;
static std::mutex globalMutex;

int lab2_open(const char* path, size_t cache_size, size_t page_size)
{
  page_size = ((page_size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;

  Storage* storage = new Storage(path);

  int result = storage->open(O_RDWR | O_CREAT | O_DIRECT, 0644);
  if (result < 0)
  {
    delete storage;
    return result;
  }

  std::lock_guard<std::mutex> lock(globalMutex);
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
  std::lock_guard<std::mutex> lock(globalMutex);

  auto it = g_storages.find(fd);
  if (it == g_storages.end())
  {
    return -1;
  }

  Storage* storage = it->second;
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

ssize_t lab2_read(int fd, void* buf, size_t count)
{
  if (!g_cache || !buf || count == 0) return -1;

  std::lock_guard<std::mutex> lock(globalMutex);
  auto it_storage = g_storages.find(fd);
  auto it_offset = g_offsets.find(fd);
  if (it_storage == g_storages.end() || it_offset == g_offsets.end())
  {
    return -1;
  }

  Storage* storage = it_storage->second;
  off_t& offset = it_offset->second;
  size_t page_size = g_cache->getPageSize();

  char* out_buf = static_cast<char*>(buf);
  size_t total_read = 0;
  LOG("lab2_read: start, fd=" << fd << ", offset=" << offset
                              << ", count=" << count);
  while (total_read < count)
  {
    size_t current_offset = offset + total_read;
    size_t bytes_to_read = std::min(page_size, count - total_read);
    LOG("lab2_read: loop start, current_offset="
        << current_offset << ", bytes_to_read=" << bytes_to_read
        << ", total_read=" << total_read);
    ssize_t read_result = g_cache->read(storage, out_buf + total_read,
                                        bytes_to_read, current_offset);
    if (read_result < 0)
    {
      LOG("lab2_read: g_cache->read returned error or 0, read_result="
          << read_result);
      return read_result;
    }
    if (read_result == 0)
    {
      LOG("lab2_read: g_cache->read returned 0, breaking");
      break;
    }
    total_read += read_result;
    LOG("lab2_read: loop end, read_result=" << read_result
                                            << ", total_read=" << total_read);
  }

  offset += total_read;
  LOG("lab2_read: end, total_read=" << total_read << ", offset=" << offset);
  return total_read;
}

ssize_t lab2_write(int fd, const void* buf, size_t count)
{
  if (g_cache == nullptr) return -1;

  std::lock_guard<std::mutex> lock(globalMutex);

  auto itStorage = g_storages.find(fd);
  auto itOffset = g_offsets.find(fd);
  if (itStorage == g_storages.end() || itOffset == g_offsets.end())
  {
    return -1;
  }

  Storage* storage = itStorage->second;
  off_t& offset = itOffset->second;
  size_t page_size = g_cache->getPageSize();

  char* aligned_buf = nullptr;
  if (posix_memalign((void**)&aligned_buf, BLOCK_SIZE, page_size) != 0)
  {
    return -1;
  }

  const char* in_buf = static_cast<const char*>(buf);
  size_t total_written = 0;

  while (total_written < count)
  {
    size_t current_offset = offset + total_written;
    size_t page_aligned_offset = (current_offset / page_size) * page_size;
    size_t offset_in_page = current_offset - page_aligned_offset;
    size_t bytes_to_write =
        std::min(page_size - offset_in_page, count - total_written);

    if (offset_in_page != 0 || bytes_to_write != page_size)
    {
      ssize_t read_result =
          g_cache->read(storage, aligned_buf, page_size, page_aligned_offset);
      if (read_result < 0 && read_result != -ENOENT)
      {
        free(aligned_buf);
        return read_result;
      }
    }

    memcpy(aligned_buf + offset_in_page, in_buf + total_written,
           bytes_to_write);

    ssize_t write_result =
        g_cache->write(storage, aligned_buf, page_size, page_aligned_offset);
    if (write_result < 0)
    {
      free(aligned_buf);
      return write_result;
    }

    total_written += bytes_to_write;
  }

  free(aligned_buf);
  offset += total_written;
  return total_written;
}

off_t lab2_lseek(int fd, off_t offset, int whence)
{
  std::lock_guard<std::mutex> lock(globalMutex);

  auto itStorage = g_storages.find(fd);
  if (itStorage == g_storages.end())
  {
    return -1;
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
  std::lock_guard<std::mutex> lock(globalMutex);
  auto it = g_storages.find(fd);
  if (it == g_storages.end())
  {
    return -1;
  }
  return g_cache->sync(it->second);
}

size_t lab2_get_page_size()
{
  std::lock_guard<std::mutex> lock(globalMutex);
  if (g_cache)
  {
    return g_cache->getPageSize();
  }
  else
  {
    return BLOCK_SIZE;
  }
}

size_t lab2_get_cache_stats(int fd, size_t* hits, size_t* misses)
{
  if (g_cache == nullptr || !hits || !misses)
  {
    return 0;
  }
  std::lock_guard<std::mutex> lock(globalMutex);
  *hits = g_cache->getHits();
  *misses = g_cache->getMisses();
  return 1;
}

void lab2_clear_cache(int fd)
{
  if (g_cache == nullptr)
  {
    return;
  }
  std::lock_guard<std::mutex> lock(globalMutex);

  auto it = g_storages.find(fd);
  if (it != g_storages.end())
  {
    g_cache->close(it->second);
  }
}

int lab2_preload(int fd, off_t offset, size_t count)
{
  if (g_cache == nullptr)
  {
    return -1;
  }
  std::lock_guard<std::mutex> lock(globalMutex);
  auto it = g_storages.find(fd);
  if (it == g_storages.end())
  {
    return -1;
  }

  Storage* storage = it->second;

  size_t page_size = g_cache->getPageSize();

  size_t total_preloaded = 0;

  while (total_preloaded < count)
  {
    size_t current_offset = offset + total_preloaded;
    Page* page = g_cache->getPage(storage, current_offset);
    if (!page)
    {
      return -1;
    }
    total_preloaded += page_size;
  }

  return 0;
}