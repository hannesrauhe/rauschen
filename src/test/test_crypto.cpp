#include <gtest/gtest.h>

#include "../common.hpp"
#include "../crypto.hpp"

TEST(CryptoTest, SignVerify) {
  Crypto c(RAUSCHEN_KEY_FILE);
  std::string str = "Some random data";
  auto sig = c.sign(str);
  ASSERT_TRUE(c.verify(str, sig, c.getPubKey()));
  str="other data";
  ASSERT_FALSE(c.verify(str, sig, c.getPubKey()));
}
TEST(CryptoTest, EncryptDecrypt) {
  Crypto c(RAUSCHEN_KEY_FILE);
  std::string str = "Some random data";
  auto cipher = c.encrypt(str, c.getPubKey());
  ASSERT_TRUE(str != cipher);
  ASSERT_TRUE(str == c.decrypt(cipher));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
