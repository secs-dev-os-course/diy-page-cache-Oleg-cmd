#include <diy_cache/storage.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>
#include <cstring> 

TEST(StorageTest, ReadWrite)
{
  const std::string TEST_FILE = "test_storage.bin";
  Storage storage(TEST_FILE);
  ASSERT_EQ(storage.open(O_RDWR | O_CREAT), 0);

  const char* test_data = "Test data";
  size_t data_size = strlen(test_data);
  ASSERT_EQ(storage.write(0, test_data, data_size), data_size);

  char read_buffer[1024];
  std::memset(read_buffer, 0, sizeof(read_buffer)); // Заполняем буфер нулями
  ASSERT_EQ(storage.read(0, read_buffer, data_size), data_size);
  ASSERT_STREQ(read_buffer, test_data);

  std::memset(read_buffer, 0, sizeof(read_buffer));
  ASSERT_EQ(storage.read(0, read_buffer, 5), 5);
  ASSERT_STREQ(read_buffer, "Test ");

  ASSERT_EQ(storage.close(), 0);
  ::remove(TEST_FILE.c_str());
}

TEST(StorageTest, ReadWriteOffset)
{
  const std::string TEST_FILE = "test_storage_offset.bin";
  Storage storage(TEST_FILE);
  ASSERT_EQ(storage.open(O_RDWR | O_CREAT), 0);

  const char* test_data1 = "Test data 1";
  size_t data_size1 = strlen(test_data1);
  ASSERT_EQ(storage.write(0, test_data1, data_size1), data_size1);

  const char* test_data2 = "Test data 2";
  size_t data_size2 = strlen(test_data2);
  ASSERT_EQ(storage.write(4096, test_data2, data_size2),
            data_size2);

  char read_buffer[1024];
  std::memset(read_buffer, 0, sizeof(read_buffer));

  ASSERT_EQ(storage.read(0, read_buffer, data_size1), data_size1);
  ASSERT_STREQ(read_buffer, test_data1);

  std::memset(read_buffer, 0, sizeof(read_buffer));
  ASSERT_EQ(storage.read(4096, read_buffer, data_size2),
            data_size2);
  ASSERT_STREQ(read_buffer, test_data2);

  ASSERT_EQ(storage.close(), 0);
  ::remove(TEST_FILE.c_str());
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}