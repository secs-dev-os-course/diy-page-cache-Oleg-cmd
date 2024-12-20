#ifndef LAB2_API_H
#define LAB2_API_H

#include <sys/types.h>
#include <unistd.h>

#include <cstddef>

#include "config.h"

int lab2_open(const char* path, size_t cache_size = 10,
              size_t page_size = 1024);
int lab2_close(int fd);
ssize_t lab2_read(int fd, void* buf, size_t count);
ssize_t lab2_write(int fd, const void* buf, size_t count);
off_t lab2_lseek(int fd, off_t offset, int whence);
int lab2_fsync(int fd);
size_t lab2_get_page_size();
size_t lab2_get_cache_stats(int fd, size_t* hits, size_t* misses);
void lab2_clear_cache(int fd);
int lab2_preload(int fd, off_t offset, size_t count);

#endif