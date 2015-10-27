#include <gtest/gtest.h>

#include "common.hpp"
#include "daemon/crypto.hpp"

class CryptoTest : public ::testing::Test {
protected:
  virtual void SetUp()
  {
    c1_.reset(createCrypto( RAUSCHEN_KEY_FILE ));
    c2_.reset(createCrypto( "testkey" ));
  }

  virtual void TearDown()
  {
  }

  Crypto* createCrypto(std::string file_name) {
    std::ifstream f( file_name );
    if(!f.good()) {
      Crypto::generate( file_name );
    }
    return new Crypto(file_name);
  }

protected:
  std::unique_ptr<Crypto> c1_;
  std::unique_ptr<Crypto> c2_;
  std::string test_str =
      "The path of the righteous man is beset on all sides by the iniquities "
      "of the selfish and the tyranny of evil men. Blessed is he who, in the "
      "name of charity and good will, shepherds the weak through the valley of"
      " darkness, for he is truly his brother's keeper and the finder of lost "
      "children. And I will strike down upon thee with great vengeance and "
      "furious anger those who would attempt to poison and destroy "
      "My brothers. And you will know My name is the Lord when I lay My vengeance upon thee.";
};

TEST_F(CryptoTest, SignVerify)
{
  auto sig = c1_->sign( test_str );
  ASSERT_TRUE( c1_->verify( test_str, sig, c1_->getPubKey() ) );
  std::string str = "other data";
  ASSERT_FALSE( c1_->verify( str, sig, c1_->getPubKey() ) );
}

TEST_F(CryptoTest, SymEncryptDecrypt)
{
  std::array<byte,AES::DEFAULT_KEYLENGTH> key;
  std::array<byte,AES::BLOCKSIZE> iv;
  std::string cipher, encoded, recovered;

  cipher = c1_->encryptSymmetrically(test_str, key, iv);
  std::string encryptedKey = c1_->encryptKey(key, c1_->getPubKey());
  ASSERT_TRUE(0 == std::memcmp( key.data(), c1_->decryptKey(encryptedKey).data(), key.size()) );
  ASSERT_TRUE(test_str == c1_->decryptSymmetrically(cipher, key, iv) );
}


TEST_F(CryptoTest, KeyEncryptDecrypt)
{
  std::array<byte, AES::DEFAULT_KEYLENGTH> key;
  std::string encryptedKey = c1_->encryptKey(key, c2_->getPubKey());
  ASSERT_TRUE(0 == std::memcmp( key.data(), c2_->decryptKey(encryptedKey).data(), key.size()) );
}


TEST_F(CryptoTest, ContainerEncrpytDecrypt)
{
  PInnerContainer inner_cont;
  inner_cont.set_message(test_str);
  auto cont = c1_->createEncryptedContainer(c2_->getPubKey(), inner_cont);

  std::unique_ptr<PInnerContainer> r_inner_cont;
  ASSERT_TRUE( c2_->checkAndEncrypt(cont, r_inner_cont) );
  ASSERT_TRUE(test_str == r_inner_cont->message());
}
