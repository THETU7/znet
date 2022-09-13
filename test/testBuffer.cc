#include "Buffer.h"
#include "gtest/gtest.h"
#include <cstring>

TEST(TESTBUFFER, FUNCTIONTEST) {
  znet::buffer::Buffer buffer;
  buffer.append("just a test");
  EXPECT_EQ(buffer.readAbleBytes(), 11);
  std::string str = buffer.retrieveAsString();
  EXPECT_EQ(str, "just a test");
  EXPECT_EQ(buffer.readAbleBytes(), 0);
  EXPECT_EQ(buffer.writeAbleBytes(), 1024);
  char buf[20498];
  memset(buf, 97, sizeof buf);
  buffer.append(buf, sizeof buf);
  EXPECT_EQ(buffer.readAbleBytes(), sizeof buf);
  const char *re = buffer.peek();
  buffer.retrieveAll();
  buffer.append("test");
  EXPECT_EQ(buffer.readAbleBytes(), 4);
  std::string result = buffer.retrieveAsString();
  EXPECT_EQ(result, std::string("test"));
  buffer.prepend("ab", 2);
  EXPECT_EQ(buffer.readAbleBytes(), 2);
  EXPECT_EQ(buffer.preAbleBytes(), buffer.kCheapPrepend - 2);
  // EXPECT_STREQ(re, buf);
}
