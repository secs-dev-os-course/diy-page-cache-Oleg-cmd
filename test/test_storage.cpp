#include <diy_cache/storage.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

TEST(StorageTest, ReadWrite)
{
  const std::string TEST_FILE = "test_storage.bin";
  Storage storage(TEST_FILE);
  ASSERT_EQ(storage.open(O_RDWR | O_CREAT | O_DIRECT), 0);

  const char* test_data = "Test data";
  size_t data_size = strlen(test_data);
  size_t aligned_size =
      ((data_size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;

  void* aligned_test_data;
  if (posix_memalign(&aligned_test_data, BLOCK_SIZE, aligned_size) != 0)
  {
    ASSERT_TRUE(false) << "posix_memalign failed for test data";
  }
  std::memcpy(aligned_test_data, test_data, data_size);

  ASSERT_EQ(storage.write(0, aligned_test_data, aligned_size), aligned_size);

  void* read_buffer;
  if (posix_memalign(&read_buffer, BLOCK_SIZE, aligned_size) != 0)
  {
    ASSERT_TRUE(false) << "posix_memalign failed for read buffer";
  }
  std::memset(read_buffer, 0, aligned_size);

  ASSERT_EQ(storage.read(0, read_buffer, aligned_size), aligned_size);

  ASSERT_STREQ(static_cast<char*>(read_buffer), test_data);

  std::memset(read_buffer, 0, aligned_size);
  size_t aligned_size_5 = ((5 + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
  ASSERT_EQ(storage.read(0, read_buffer, aligned_size_5), aligned_size_5);
  char buf[6] = {'\0'};
  std::strncpy(buf, static_cast<char*>(read_buffer), 5);
  ASSERT_STREQ(buf, "Test ");

  ASSERT_EQ(storage.close(), 0);
  free(aligned_test_data);
  free(read_buffer);
  ::remove(TEST_FILE.c_str());
}

TEST(StorageTest, ReadWriteOffset)
{
  const std::string TEST_FILE = "test_storage_offset.bin";
  Storage storage(TEST_FILE);
  ASSERT_EQ(storage.open(O_RDWR | O_CREAT | O_DIRECT), 0);

  size_t data_size1 =
      ((strlen("Test data 1") + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
  void* aligned_test_data1;
  if (posix_memalign(&aligned_test_data1, BLOCK_SIZE, data_size1) != 0)
  {
    ASSERT_TRUE(false) << "posix_memalign failed for test data1";
  }
  std::memcpy(aligned_test_data1, "Test data 1", strlen("Test data 1"));

  ASSERT_EQ(storage.write(0, aligned_test_data1, data_size1), data_size1);

  size_t data_size2 =
      ((strlen("Test data 2") + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;

  void* aligned_test_data2;
  if (posix_memalign(&aligned_test_data2, BLOCK_SIZE, data_size2) != 0)
  {
    ASSERT_TRUE(false) << "posix_memalign failed for test data2";
  }
  std::memcpy(aligned_test_data2, "Test data 2", strlen("Test data 2"));

  ASSERT_EQ(storage.write(4096, aligned_test_data2, data_size2), data_size2);

  void* read_buffer;
  if (posix_memalign(&read_buffer, BLOCK_SIZE, data_size1) != 0)
  {
    ASSERT_TRUE(false) << "posix_memalign failed for read_buffer";
  }
  std::memset(read_buffer, 0, data_size1);

  ASSERT_EQ(storage.read(0, read_buffer, data_size1), data_size1);
  ASSERT_STREQ(static_cast<char*>(read_buffer), "Test data 1");

  std::memset(read_buffer, 0, data_size2);
  if (posix_memalign(&read_buffer, BLOCK_SIZE, data_size2) != 0)
  {
    ASSERT_TRUE(false) << "posix_memalign failed for read_buffer";
  }

  ASSERT_EQ(storage.read(4096, read_buffer, data_size2), data_size2);

  ASSERT_STREQ(static_cast<char*>(read_buffer), "Test data 2");
  free(aligned_test_data1);
  free(aligned_test_data2);
  free(read_buffer);

  ASSERT_EQ(storage.close(), 0);
  ::remove(TEST_FILE.c_str());
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}