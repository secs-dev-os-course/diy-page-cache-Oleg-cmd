#include <diy_cache/lab2_api.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdio>
#include <fcntl.h>

int main() {
    std::remove("test_file.dat");

    const char* path = "test_file.dat";
    int fd = lab2_open(path, O_RDWR | O_CREAT, 0666);  // Ensure correct flags for open
    if (fd < 0) {
        std::cerr << "Error opening file: " << fd << std::endl;
        return 1;
    }

    const char* data1 = "This is the first line\n";
    std::cerr << "Writing data1: " << data1; 
    ssize_t bytesWritten = lab2_write(fd, data1, strlen(data1));
    if (bytesWritten < 0) {
        std::cerr << "Error writing: " << bytesWritten << std::endl;
        lab2_close(fd);
        return 1;
    }

    // Set offset
    off_t newOffset = lab2_lseek(fd, 4096, SEEK_SET); 
    if (newOffset < 0) {
        std::cerr << "lseek error: " << newOffset << std::endl;
        lab2_close(fd);
        return 1;
    }

    const char* data2 = "This is the second line\n";
    std::cerr << "Writing data2: " << data2; 
    bytesWritten = lab2_write(fd, data2, strlen(data2));
    if (bytesWritten < 0) {
        std::cerr << "Error writing: " << bytesWritten << std::endl;
        lab2_close(fd);
        return 1;
    }

    // Synchronize
    int fsyncRes = lab2_fsync(fd);
    if (fsyncRes < 0) {
        std::cerr << "Error fsync: " << fsyncRes << std::endl;
    }

    // Check file size
    struct stat st;
    if (fstat(fd, &st) == 0) {
        std::cout << "File size after writes: " << st.st_size << std::endl;
    } else {
        std::cerr << "Error getting file size" << std::endl;
    }

    // Reset file pointer for reading
    lab2_lseek(fd, 0, SEEK_SET);

    // Dynamic buffer size: We can calculate it based on the file size or a given threshold
    off_t fileSize = st.st_size;
    size_t bufferSize = fileSize > 0 ? static_cast<size_t>(fileSize) : 8192;  // Default to 8KB if file is empty
    char* buf = new char[bufferSize];  // Create buffer dynamically based on file size

    ssize_t bytesRead = lab2_read(fd, buf, bufferSize);
    if (bytesRead < 0) {
        std::cerr << "Error reading: " << bytesRead << std::endl;
        delete[] buf;
        lab2_close(fd);
        return 1;
    }

    std::cout << "Buffer contents: ";
    for (ssize_t i = 0; i < bytesRead; ++i) {
        std::cout << buf[i];
    }
    std::cout << std::endl;

    // Log read bytes
    std::cout << "Bytes read: " << bytesRead << std::endl;
    if (bytesRead > 0) {
        std::cout.write(buf, bytesRead);
        std::cout << std::endl;
    } else {
        std::cerr << "No data read or end of file reached." << std::endl;
    }

    // Clean up
    delete[] buf;
    lab2_close(fd);
    std::remove("test_file.dat");

    return 0;
}
