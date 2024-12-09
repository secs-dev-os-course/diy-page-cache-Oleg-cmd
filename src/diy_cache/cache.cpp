#include <diy_cache/cache.h>
#include <diy_cache/storage.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cstring>

#ifdef DEBUG_CACHE
#include <iostream>
#endif

#ifdef DEBUG_CACHE
#define LOG(message) std::cerr << "[Cache] " << message << std::endl
#else
#define LOG(message)
#endif

Cache::Cache(PageSize pageSize, CacheSize cacheSize)
    : cacheSize(cacheSize), pageSize(pageSize)
{
  LOG("Constructor: cacheSize=" << cacheSize.getValue()
                                << ", pageSize=" << pageSize.getValue());
}

Cache::~Cache() {
    pageMap.clear();
}

auto Cache::read(Storage* storage, void* buf, size_t count,
                 off_t offset) -> ssize_t
{
  LOG("read: fd=" << storage->getFd() << ", offset=" << offset
                  << ", count=" << count);

  ssize_t totalBytesRead = 0;
  char* charBuf = static_cast<char*>(buf);

  // Получаем страницу из кэша
  while (count > 0)
  {
    Page* page = getPage(storage, offset);
    if (!page)
    {
      LOG("read: Error getting page");
      return -1;
    }

    if (offset < 0)
    {
      LOG("Error: Negative offset encountered");
      return -1;
    }

    size_t offsetInPage =
        static_cast<size_t>(static_cast<size_t>(offset) % pageSize.getValue());
    size_t bytesToRead = std::min({count, pageSize.getValue() - offsetInPage,
                                   page->getPageSize() - offsetInPage});

    struct stat st;
    off_t fileSize;
    if (fstat(storage->getFd(), &st) == 0)
    {
      fileSize = st.st_size;
    }
    else
    {
      perror("fstat failed");
      return -1;
    }

    if (offset >= fileSize)
    {
      return 0; // Если смещение больше размера файла, ничего не читаем
    }

    bytesToRead = std::min(bytesToRead, static_cast<size_t>(fileSize - offset));

    LOG("Cache::read: Reading from page at offset "
        << page->getOffset() << ", offsetInPage=" << offsetInPage
        << ", bytesToRead=" << bytesToRead);

    std::copy(page->getData() + offsetInPage,
              page->getData() + offsetInPage + bytesToRead, charBuf);

    LOG("Cache::read: Copied " << bytesToRead << " bytes to buffer at address "
                               << static_cast<void*>(charBuf));

    charBuf += bytesToRead;

    page->setAccessed(true);

    totalBytesRead += bytesToRead;
    count -= bytesToRead;
    offset += bytesToRead;
  }

  LOG("read: Read " << totalBytesRead << " bytes");
  return totalBytesRead;
}

ssize_t Cache::write(Storage* storage, const void* buf, size_t count,
                     off_t offset)
{
  LOG("write: fd=" << storage->getFd() << ", offset=" << offset
                   << ", count=" << count);

  Page* page = getPage(storage, offset);
  if (!page)
  {
    LOG("write: Error getting page");
    return -1;
  }

  size_t offsetInPage =
      static_cast<size_t>(static_cast<size_t>(offset) % pageSize.getValue());
  size_t bytesToWrite = std::min(count, pageSize.getValue() - offsetInPage);

  std::memcpy(page->getData() + offsetInPage, buf, bytesToWrite);

  page->setDirty(true); // Помечаем страницу как грязную
  page->setAccessed(true);

  LOG("write: Wrote " << bytesToWrite
                      << " bytes, page size=" << page->getPageSize()
                      << ", offsetInPage=" << offsetInPage);

  return static_cast<ssize_t>(bytesToWrite);
}

int Cache::sync(Storage* storage)
{
  LOG("sync: fd=" << storage->getFd() << ", num pages=" << pageList.size());
  // int dirtyPages = 0;

  for (auto& page : pageList)
  {
    if (page.isDirty())
    {
      // dirtyPages++;
      LOG("sync: Writing dirty page, offset=" << page.getOffset() << ", size="
                                              << page.getPageSize());

      ssize_t bytesWritten =
          storage->write(page.getOffset(), page.getData(), page.getPageSize());

      if (bytesWritten == -1)
      {
        LOG("sync: Error writing dirty page, errno: " << strerror(errno));
        return -1;
      }

      LOG("sync: Bytes written: " << bytesWritten);

      page.setDirty(false);
    }
  }
  LOG("sync: Synced " << " dirty pages");
  return 0;
}

Page* Cache::getPage(Storage* storage, off_t offset)
{
  off_t pageOffset = static_cast<off_t>(
      static_cast<size_t>(offset) / static_cast<size_t>(pageSize.getValue()) *
      pageSize.getValue());

  LOG("getPage: fd=" << storage->getFd() << ", offset=" << offset
                     << ", pageOffset=" << pageOffset);

  auto it = pageMap.find(pageOffset);
  if (it != pageMap.end())
  {
    LOG("getPage: Page found in cache");

    pageList.splice(pageList.end(), pageList, it->second);
    pageMap[pageOffset] = std::prev(pageList.end());

    return &(*pageMap[pageOffset]);
  }

  LOG("getPage: Page not found in cache");

  if (pageList.size() >= cacheSize.getValue())
  {
    LOG("getPage: Cache full, evicting");
    evict(storage);
  }

  pageList.emplace_back(pageSize.getValue());
  Page* newPage = &pageList.back();

  LOG("getPage: Reading from storage at offset " << pageOffset);
  ssize_t bytesRead =
      storage->read(pageOffset, newPage->getData(), pageSize.getValue());

  if (bytesRead == ssize_t(-1))
  {
    LOG("getPage: Error reading from storage, errno: " << strerror(errno));
    pageList.pop_back();
    return nullptr;
  }

  if (static_cast<ssize_t>(bytesRead) <
      static_cast<ssize_t>(pageSize.getValue()))
  {
    if (bytesRead < 0)
    {
      LOG("Error: Negative bytesRead encountered");
      return nullptr;
    }
    std::memset(newPage->getData() + bytesRead, 0,
                static_cast<size_t>(pageSize.getValue()) -
                    static_cast<size_t>(bytesRead));
  }

  newPage->setOffset(pageOffset);
  pageMap[pageOffset] = std::prev(pageList.end());

  LOG("getPage: Page loaded from storage, size=" << newPage->getPageSize());

  return newPage;
}

void Cache::evict(Storage* storage)
{
  LOG("evict: fd=" << storage->getFd() << ", cache size=" << pageList.size());

  for (auto it = pageList.begin(); it != pageList.end(); ++it)
  {
    if (!it->isAccessed())
    {
      if (it->isDirty())
      {
        LOG("evict: Writing dirty page at offset " << it->getOffset());
        if (storage->write(it->getOffset(), it->getData(), it->getPageSize()) ==
            -1)
        {
          LOG("evict: Error writing dirty page");
        }
        it->setDirty(false);
      }

      LOG("evict: Evicting page at offset " << it->getOffset());
      pageMap.erase(it->getOffset());
      pageList.erase(it);
      return;
    }
    else
    {
      it->setAccessed(false);
    }
  }

  // Если все страницы были accessed, вытесняем первую
  auto it = pageList.begin();
  if (it->isDirty())
  {
    LOG("evict: All pages accessed. Writing dirty page at offset "
        << it->getOffset());
    if (storage->write(it->getOffset(), it->getData(), it->getPageSize()) == -1)
    {
      LOG("evict: Error writing dirty page");
    }
  }

  LOG("evict: Evicting page at offset " << it->getOffset());
  pageMap.erase(it->getOffset());
  pageList.pop_front();
}

void Cache::close(Storage* storage)
{
  LOG("close: fd=" << storage->getFd());

  for (auto& page : pageList)
  {
    if (page.isDirty())
    {
      LOG("close: Writing dirty page at offset " << page.getOffset());

      if (storage->write(page.getOffset(), page.getData(),
                         page.getPageSize()) == -1)
      {
        LOG("close: Error writing dirty page");
      }

      page.setDirty(false);
    }
  }

  pageList.clear();
  pageMap.clear();
}
