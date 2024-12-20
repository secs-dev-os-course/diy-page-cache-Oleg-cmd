#ifndef CACHE_H
#define CACHE_H

#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <list>
#include <mutex>
#include <unordered_map>

#include "config.h"
#include "page.h"
#include "storage.h"

struct CacheSize
{
 private:
  size_t value;

 public:
  explicit CacheSize(size_t size) : value(size) {}
  [[nodiscard]] auto getValue() const -> size_t { return value; }
};

struct PageSize
{
 private:
  size_t value;

 public:
  explicit PageSize(size_t size) : value(size) {}

  [[nodiscard]] auto getValue() const -> size_t { return value; }

  [[nodiscard]] auto getPageSize() const -> size_t { return this->getValue(); }
};

class Cache
{
 public:
  Cache(PageSize pageSize, CacheSize cacheSize);
  ~Cache();
  void close(Storage* storage);

  auto read(Storage* storage, void* buf, size_t count, off_t offset) -> ssize_t;
  auto write(Storage* storage, const void* buf, size_t count,
             off_t offset) -> ssize_t;
  auto sync(Storage* storage) -> int;

  auto getPage(Storage* storage, off_t offset) -> Page*;

  [[nodiscard]] auto getPageSize() const -> size_t
  {
    return pageSize.getValue();
  }

  [[nodiscard]] size_t getHits() const { return hits; }
  [[nodiscard]] size_t getMisses() const { return misses; }
  [[nodiscard]] size_t getCacheSize() const { return cacheSize.getValue(); }

 private:
  CacheSize cacheSize;
  PageSize pageSize;
  std::list<Page> pageList;
  std::unordered_map<off_t, std::list<Page>::iterator> pageMap;
  size_t hits{0};
  size_t misses{0};
  std::mutex cacheMutex; // Mutex for protecting cache
  void evict(Storage* storage);
};

#endif