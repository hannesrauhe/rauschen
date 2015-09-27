#pragma once

#include "logger.hpp"
#include <cryptopp/rsa.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/files.h>
#include <cryptopp/pssr.h>
#include <cryptopp/osrng.h>
#include <cryptopp/secblock.h>
#include <cryptopp/cryptlib.h>

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


#include <string>
#include <exception>
#include <iostream>
#include <assert.h>

using PubKey = std::string;

class Crypto
{
public:
  static void generate( const std::string& fname )
  {
    Logger::info("Creating key pair");
    AutoSeededRandomPool rng;

    RSA::PrivateKey privateKey;
    privateKey.GenerateRandomWithKeySize(rng, 3072);

    RSA::PublicKey publicKey(privateKey);
    save(fname+".pub", publicKey);
    save(fname, privateKey);
  }

  template<class KeyT>
  static void save(const std::string& filename, const KeyT& key)
  {
    CryptoPP::ByteQueue bt;
    key.Save(bt);
    FileSink file(filename.c_str());

    bt.CopyTo(file);
    file.MessageEnd();
  }

  template<class KeyT>
  static void load(const std::string& filename, KeyT& key, std::string* keystr = nullptr)
  {
    CryptoPP::ByteQueue bt;
    FileSource file(filename.c_str(), true /*pumpAll*/);

    file.TransferTo(bt);
    bt.MessageEnd();
    if(keystr!=nullptr) {
      StringSink s(*keystr);
      bt.CopyTo(s);
    }
    key.Load(bt);
  }

  static std::string getFingerprint( const std::string& key_str )
  {
    SHA1 hash;
    byte buffer[2 * SHA1::DIGESTSIZE]; // Output size of the buffer

    StringSource f(key_str, true,
                 new CryptoPP::HashFilter(hash,
                 new CryptoPP::HexEncoder(new CryptoPP::ArraySink(buffer,2 * SHA1::DIGESTSIZE))));

    return std::string((const char*)buffer,2 * SHA1::DIGESTSIZE);
  }

  Crypto( const std::string& key_file_path )
  {
    load(key_file_path+".pub", pubk_, &pubk_str_);
    load(key_file_path, privk_);
    fingerprint_ = getFingerprint(pubk_str_);
  }

  const std::string& getPubKey() const
  {
    return pubk_str_;
  }

  std::string sign( const std::string& data) {
    std::string signature;
    CryptoPP::RSASS<PSS, SHA1>::Signer signer(privk_);

    StringSource ss1(data, true,
        new CryptoPP::SignerFilter(rng_, signer,
            new StringSink(signature)
      ) // SignerFilter
    ); // StringSource
    return signature;
  }

  bool verify( const std::string& data, const std::string& signature_str, const std::string& other_key) {
    StringSource k(other_key, true);
    RSA::PublicKey pubkey;
    pubkey.Load(k);
    CryptoPP::RSASS<PSS, SHA1>::Verifier verifier(pubkey);

    try {
      StringSource ss2(data+signature_str, true,
          new CryptoPP::SignatureVerificationFilter(
              verifier,
              nullptr,
              CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION |
              CryptoPP::SignatureVerificationFilter::PUT_MESSAGE
        ) // SignatureVerificationFilter
      ); // StringSource
    } catch( const CryptoPP::SignatureVerificationFilter::SignatureVerificationFailed& e ) {
      return false;
    }
    return true;
  }

  std::string encrypt( const std::string& plain, const std::string& other_key_str )
  {
    std::string cipher;
    RSA::PublicKey key;
    CryptoPP::ByteQueue bt;
    StringSource s(other_key_str, true, &bt);
    key.Load(bt);

    RSAES_OAEP_SHA_Encryptor e( pubk_ );

    StringSource( plain, true,
        new PK_EncryptorFilter( rng_, e,
            new StringSink( cipher )
        ) // PK_EncryptorFilter
     ); // StringSource

    return cipher;
  }

  std::string decrypt( const std::string& cipher )
  {
    std::string plain;
    RSAES_OAEP_SHA_Decryptor d( privk_ );

    StringSource( cipher, true,
        new PK_DecryptorFilter( rng_, d,
            new StringSink( plain )
        ) // PK_EncryptorFilter
     ); // StringSource
    return plain;
  }

protected:
  AutoSeededRandomPool rng_;
  RSA::PublicKey pubk_;
  RSA::PrivateKey privk_;
  std::string pubk_str_;
  std::string fingerprint_;
};
