#pragma once

#include "logger.hpp"
#include "message.pb.h"
#include <cryptopp/rsa.h>
#include <cryptopp/aes.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/files.h>
#include <cryptopp/pssr.h>
#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/modes.h>

using CryptoPP::Exception;
using CryptoPP::DecodingResult;
using CryptoPP::RSA;
using CryptoPP::InvertibleRSAFunction;
using CryptoPP::RSAES_OAEP_SHA_Encryptor;
using CryptoPP::RSAES_OAEP_SHA_Decryptor;
using CryptoPP::FileSink;
using CryptoPP::FileSource;
using CryptoPP::PSS;
using CryptoPP::SecByteBlock;
using CryptoPP::AutoSeededRandomPool;
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;
using CryptoPP::SHA1;
using CryptoPP::AES;
using CryptoPP::CFB_Mode;
using CryptoPP::StreamTransformationFilter;

#include <string>
#include <exception>
#include <iostream>
#include <assert.h>

class Crypto
{
public:
  static void generate( const std::string& fname );

  template<class KeyT>
  static void save( const std::string& filename, const KeyT& key )
  {
    CryptoPP::ByteQueue bt;
    key.Save( bt );
    FileSink file( filename.c_str() );

    bt.CopyTo( file );
    file.MessageEnd();
  }

  template<class KeyT>
  static void load( const std::string& filename, KeyT& key, std::string* keystr = nullptr )
  {
    CryptoPP::ByteQueue bt;
    FileSource file( filename.c_str(), true /*pumpAll*/);

    file.TransferTo( bt );
    bt.MessageEnd();
    if ( keystr != nullptr )
    {
      StringSink s( *keystr );
      bt.CopyTo( s );
    }
    key.Load( bt );
  }

  static std::string getFingerprint( const std::string& key_str );

public:
  Crypto( const std::string& key_file_path );

  const std::string& getPubKey() const
  {
    return pubk_str_;
  }

  bool checkAndEncrypt( const PEncryptedContainer& outer, std::unique_ptr<PInnerContainer>& inner_container );

  PEncryptedContainer createEncryptedContainer( const std::string& receiver = "", const PInnerContainer& inner_cont =
      PInnerContainer() );

//protected:
  std::string sign( const std::string& data );

  bool verify( const std::string& data, const std::string& signature_str, const std::string& other_key );

  std::string encryptKey( const std::array<byte, AES::DEFAULT_KEYLENGTH>& sym_key, const std::string& other_key_str );

  std::array<byte, AES::DEFAULT_KEYLENGTH> decryptKey( const std::string& cipher );

  std::string encryptSymmetrically( const std::string& plain, std::array<byte, AES::DEFAULT_KEYLENGTH>& key,
      std::array<byte, AES::BLOCKSIZE>& iv );

  std::string decryptSymmetrically( const std::string& cipher, const std::array<byte, AES::DEFAULT_KEYLENGTH>& key,
      const std::array<byte, AES::BLOCKSIZE>& iv );

protected:
  AutoSeededRandomPool rng_;
  RSA::PublicKey pubk_;
  RSA::PrivateKey privk_;
  std::string pubk_str_;
  std::string fingerprint_;
};
