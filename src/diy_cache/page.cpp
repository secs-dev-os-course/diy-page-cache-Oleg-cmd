#include <diy_cache/page.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

Page::Page(size_t pageSize)
    : offset(0),
      data(nullptr),
      pageSize(pageSize),
      dirty(false),
      accessed(false)
{
  if (posix_memalign((void**)&data, BLOCK_SIZE, pageSize) != 0)
  {
    throw std::runtime_error("Failed to allocate aligned memory for page");
  }
}

Page::~Page() { free(data); }

Page::Page(Page&& other) noexcept
    : offset(other.offset),
      data(other.data),
      pageSize(other.pageSize),
      dirty(other.dirty),
      accessed(other.accessed)
{
  other.data = nullptr;
  other.pageSize = 0;
}

Page& Page::operator=(Page&& other) noexcept
{
  if (this != &other)
  {
    free(data);

    offset = other.offset;
    data = other.data;
    pageSize = other.pageSize;
    dirty = other.dirty;
    accessed = other.accessed;

    other.data = nullptr;
    other.pageSize = 0;
  }
  return *this;
}

bool Page::isAligned() const
{
  return (reinterpret_cast<uintptr_t>(data) % BLOCK_SIZE == 0) &&
         (pageSize % BLOCK_SIZE == 0);
}

size_t Page::getAlignedSize() const
{
  return ((pageSize + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
}

void Page::realign()
{
  if (isAligned()) return;

  void* new_data = nullptr;
  size_t aligned_size = getAlignedSize();

  if (posix_memalign(&new_data, BLOCK_SIZE, aligned_size) != 0)
  {
    throw std::runtime_error("Failed to allocate aligned memory");
  }

  if (data)
  {
    std::memcpy(new_data, data, pageSize);
    free(data);
  }

  data = static_cast<char*>(new_data);
  pageSize = aligned_size;
}