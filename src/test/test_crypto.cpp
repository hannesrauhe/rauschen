#include <gtest/gtest.h>
#include "../crypto.hpp"

TEST(CryptoTest, SignVerify) {
  Crypto c("test.key");
  std::string str = "Some random data";
  auto sig = c.sign(str);
  ASSERT_TRUE(c.verify(str, sig, c.getPubKey()));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
