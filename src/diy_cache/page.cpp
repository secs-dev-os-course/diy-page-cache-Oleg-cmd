#include <diy_cache/page.h>

#include <cstring>

Page::Page(size_t pageSize)
    : offset(-1),
      data(new char[pageSize]),
      pageSize(pageSize),
      dirty(false),
      accessed(false)
{
  std::memset(data, 0, pageSize);
}

Page::~Page() { delete[] data; }