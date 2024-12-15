#ifndef PAGE_H
#define PAGE_H

#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include "config.h"

class Page
{
public:
    Page(size_t pageSize = 1024);
    ~Page();

    [[nodiscard]] auto getOffset() const -> off_t { return offset; }
    void setOffset(off_t off) { offset = off; }
    auto getData() -> char* { return data; }
    [[nodiscard]] auto getData() const -> const char* { return data; }
    [[nodiscard]] auto getPageSize() const -> size_t { return pageSize; }
    [[nodiscard]] auto isDirty() const -> bool { return dirty; }
    void setDirty(bool d) { dirty = d; }
    [[nodiscard]] auto isAccessed() const -> bool { return accessed; }
    void setAccessed(bool a) { accessed = a; }
    [[nodiscard]] bool isAligned() const;
    [[nodiscard]] size_t getAlignedSize() const;
    void realign();


    private:
    off_t offset;
    char* data;
    size_t pageSize;
    bool dirty;
    bool accessed;

    // Запрещаем копирование, так как управляем выровненной памятью
    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;
    
    // Разрешаем перемещение
    Page(Page&&) noexcept;
    Page& operator=(Page&&) noexcept;

};

#endif