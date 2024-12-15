#include <diy_cache/cache.h>
#include <diy_cache/storage.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdexcept>
#include <algorithm>
#include <cstring>

#ifdef DEBUG_CACHE
#include <iostream>
#endif


Cache::Cache(PageSize pageSize, CacheSize cacheSize)
    : cacheSize(cacheSize), 
      pageSize(pageSize),
      hits(0),
      misses(0)
{
    if (pageSize.getValue() % BLOCK_SIZE != 0) {
        throw std::invalid_argument("Page size must be a multiple of BLOCK_SIZE");
    }
    
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
  
  off_t current_offset = offset;

  while (count > 0)
  {
    LOG("Cache::read: loop start, current_offset=" << current_offset << ", count=" << count);  
    Page* page = getPage(storage, current_offset);
    if (!page)
    {
      LOG("read: Error getting page");
      return -1;
    }

    if (current_offset < 0)
    {
      LOG("Error: Negative offset encountered");
      return -1;
    }

    size_t offsetInPage =
        static_cast<size_t>(static_cast<size_t>(current_offset) % pageSize.getValue());
    size_t bytesToRead = std::min({count, pageSize.getValue() - offsetInPage,
                                   page->getPageSize() - offsetInPage});
   
    if(bytesToRead == 0){
        LOG("Cache::read: break condition, bytesToRead=" << bytesToRead);
        break;
    }
    LOG("Cache::read: Reading from page at offset "
        << page->getOffset() << ", offsetInPage=" << offsetInPage
        << ", bytesToRead=" << bytesToRead);

   memcpy(charBuf, page->getData() + offsetInPage, bytesToRead);

    LOG("Cache::read: Copied " << bytesToRead << " bytes to buffer at address "
                               << static_cast<void*>(charBuf));

    charBuf += bytesToRead;

    page->setAccessed(true);

    totalBytesRead += bytesToRead;
    count -= bytesToRead;
    current_offset += bytesToRead;
    
    LOG("Cache::read: loop end, totalBytesRead=" << totalBytesRead << ", current_offset=" << current_offset << ", count=" << count);  
  }

  LOG("read: Read " << totalBytesRead << " bytes");
  return totalBytesRead;
}

ssize_t Cache::write(Storage* storage, const void* buf, size_t count, off_t offset) {
    if (!storage || !buf || count == 0) return -1;

    Page* page = getPage(storage, offset);
    if (!page) return -1;

    size_t bytes_to_write = std::min(count, page->getPageSize());
    memcpy(page->getData(), buf, bytes_to_write);
    
    page->setDirty(true);
    
    return storage->write(offset, page->getData(), page->getPageSize());
}



int Cache::sync(Storage* storage)
{
    LOG("sync: fd=" << storage->getFd() << ", num pages=" << pageList.size());

    for (auto& page : pageList)
    {
        if (page.isDirty())
        {
            size_t aligned_size = ((pageSize.getValue() + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
            void* aligned_buf = nullptr;
            
            if (posix_memalign(&aligned_buf, BLOCK_SIZE, aligned_size) != 0) {
                LOG("sync: Failed to allocate aligned memory");
                return -1;
            }

            std::memcpy(aligned_buf, page.getData(), aligned_size);
            
            LOG("sync: Writing dirty page, offset=" << page.getOffset());
            ssize_t bytesWritten = storage->write(page.getOffset(), aligned_buf, aligned_size);
            free(aligned_buf);

            if (bytesWritten < 0)
            {
                LOG("sync: Error writing dirty page: " << strerror(errno));
                return bytesWritten;
            }

            page.setDirty(false);
        }
    }
    
    if (fsync(storage->getFd()) != 0) {
        LOG("sync: fsync failed: " << strerror(errno));
        return -errno;
    }

    return 0;
}

Page* Cache::getPage(Storage* storage, off_t offset)
{
    auto it = pageMap.find(offset);
    if (it != pageMap.end()) {
        hits++;
        return &(*it->second);
    }
    
    misses++;
    
    if (pageList.size() >= cacheSize.getValue()) {
        evict(storage);
    }
    
    pageList.emplace_back(pageSize.getValue());
    Page* page = &pageList.back();
    
    ssize_t bytes_read = storage->read(offset, page->getData(), page->getPageSize());
    if (bytes_read < 0) {
        pageList.pop_back();
        return nullptr;
    }
    
    page->setOffset(offset);
    pageMap[offset] = std::prev(pageList.end());
    
    return page;
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
                
                size_t aligned_size = ((pageSize.getValue() + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
                
                if (!Storage::checkAlignment(it->getData(), it->getOffset(), aligned_size)) {
                    void* aligned_buf = nullptr;
                    if (posix_memalign(&aligned_buf, BLOCK_SIZE, aligned_size) == 0) {
                        std::memcpy(aligned_buf, it->getData(), aligned_size);
                        storage->write(it->getOffset(), aligned_buf, aligned_size);
                        free(aligned_buf);
                    }
                } else {
                    storage->write(it->getOffset(), it->getData(), aligned_size);
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

    auto it = pageList.begin();
    if (it->isDirty())
    {
        size_t aligned_size = ((pageSize.getValue() + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
        
        if (!Storage::checkAlignment(it->getData(), it->getOffset(), aligned_size)) {
            void* aligned_buf = nullptr;
            if (posix_memalign(&aligned_buf, BLOCK_SIZE, aligned_size) == 0) {
                std::memcpy(aligned_buf, it->getData(), aligned_size);
                storage->write(it->getOffset(), aligned_buf, aligned_size);
                free(aligned_buf);
            }
        } else {
            storage->write(it->getOffset(), it->getData(), aligned_size);
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