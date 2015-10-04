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

using PubKey = std::string;

class Crypto
{
public:
  static void generate( const std::string& fname )
  {
    Logger::info( "Creating key pair" );
    AutoSeededRandomPool rng;

    RSA::PrivateKey privateKey;
    privateKey.GenerateRandomWithKeySize( rng, 3072 );

    RSA::PublicKey publicKey( privateKey );
    save( fname + ".pub", publicKey );
    save( fname, privateKey );
  }

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

  static std::string getFingerprint( const std::string& key_str )
  {
    SHA1 hash;
    byte buffer[2 * SHA1::DIGESTSIZE]; // Output size of the buffer

    StringSource f( key_str, true,
        new CryptoPP::HashFilter( hash,
            new CryptoPP::HexEncoder( new CryptoPP::ArraySink( buffer, 2 * SHA1::DIGESTSIZE ) ) ) );

    return std::string( (const char*) buffer, 2 * SHA1::DIGESTSIZE );
  }

public:
  Crypto( const std::string& key_file_path )
  {
    load( key_file_path + ".pub", pubk_, &pubk_str_ );
    load( key_file_path, privk_ );
    fingerprint_ = getFingerprint( pubk_str_ );
  }

  const std::string& getPubKey() const
  {
    return pubk_str_;
  }

  bool checkAndEncrypt( const PEncryptedContainer& outer, std::unique_ptr<PInnerContainer>& inner_container )
  {
//    Logger::debug( "Received Message from " + Crypto::getFingerprint( outer.pubkey() ) + " - " + sender.to_string() );
    if ( !outer.has_pubkey() || !outer.has_sym_key() || !outer.has_sym_iv())
    {
      return false;
    }
    if ( pubk_str_ == outer.pubkey() )
    {
      Logger::debug( "Message came from myself" );
      //record ip
      return false;
    }
    if ( !outer.has_container() )
    {
      Logger::debug( "Message was a ping" );
      inner_container.reset();
      return true;
    }

    auto key = decryptKey(outer.sym_key());
    std::array<byte, AES::BLOCKSIZE> iv;
    for(auto i = 0; i<AES::BLOCKSIZE; ++i) {
      iv[i] = static_cast<unsigned char>(outer.sym_iv()[i]);
    }
    PSignedContainer sig_cont;

    sig_cont.ParseFromString( this->decryptSymmetrically( outer.container(), key, iv ) );
    inner_container.reset( sig_cont.release_inner_cont() );
    bool v_success = verify( inner_container->SerializeAsString(), sig_cont.signature(), outer.pubkey() );
    if ( !v_success ) Logger::debug( "Verification failed" );

    return v_success;
  }

  PEncryptedContainer createEncryptedContainer( const std::string& receiver = "", const PInnerContainer& inner_cont =
      PInnerContainer() )
  {
    PEncryptedContainer enc_cont;
    enc_cont.set_version( RAUSCHEN_MESSAGE_FORMAT_VERSION );
    enc_cont.set_pubkey( pubk_str_ );
    if ( !receiver.empty() )
    {
      PSignedContainer sign_cont;
      auto inner_cont_ptr = sign_cont.mutable_inner_cont();
      *inner_cont_ptr = inner_cont;
      if ( !inner_cont_ptr->has_timestamp() )
      {
        inner_cont_ptr->set_timestamp( std::chrono::seconds( std::time( nullptr ) ).count() );
      }
      if ( !inner_cont_ptr->has_receiver() )
      {
        inner_cont_ptr->set_receiver( Crypto::getFingerprint( receiver ) );
      }
      sign_cont.set_signature( sign( inner_cont_ptr->SerializeAsString() ) );

      std::array<byte, AES::DEFAULT_KEYLENGTH> key;
      std::array<byte, AES::BLOCKSIZE> iv;
      enc_cont.set_container( encryptSymmetrically( sign_cont.SerializeAsString(), key, iv ) );
      enc_cont.set_sym_key( encryptKey(key, receiver) );
      enc_cont.set_sym_iv( iv.data(), iv.size() );
    }
    return enc_cont;
  }

//protected:
  std::string sign( const std::string& data )
  {
    std::string signature;
    CryptoPP::RSASS<PSS, SHA1>::Signer signer( privk_ );

    StringSource ss1( data, true, new CryptoPP::SignerFilter( rng_, signer, new StringSink( signature ) ) // SignerFilter
        );// StringSource
    return signature;
  }

  bool verify( const std::string& data, const std::string& signature_str, const std::string& other_key )
  {
    StringSource k( other_key, true );
    RSA::PublicKey pubkey;
    pubkey.Load( k );
    CryptoPP::RSASS<PSS, SHA1>::Verifier verifier( pubkey );

    try
    {
      StringSource ss2( data + signature_str, true,
          new CryptoPP::SignatureVerificationFilter( verifier, nullptr,
              CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION
                  | CryptoPP::SignatureVerificationFilter::PUT_MESSAGE ) // SignatureVerificationFilter
                  );// StringSource
    }
    catch ( const CryptoPP::SignatureVerificationFilter::SignatureVerificationFailed& e )
    {
      return false;
    }
    return true;
  }

  std::string encryptKey( const std::array<byte, AES::DEFAULT_KEYLENGTH>& sym_key, const std::string& other_key_str )
  {
    std::string cipher;
    RSA::PublicKey key;
    CryptoPP::ByteQueue bt;
    StringSource s( other_key_str, true );
    s.CopyTo(bt);
    key.Load( bt );

    RSAES_OAEP_SHA_Encryptor e( key );
    assert( 0 != e.FixedMaxPlaintextLength() );
    assert( sym_key.size() <= e.FixedMaxPlaintextLength() );

    StringSource( sym_key.data(), sym_key.size(), true, new PK_EncryptorFilter( rng_, e, new StringSink( cipher ) )
        );
    return cipher;
  }

  std::array<byte, AES::DEFAULT_KEYLENGTH> decryptKey( const std::string& cipher )
  {
    std::string plain;
    RSAES_OAEP_SHA_Decryptor d( privk_ );

    StringSource( cipher, true, new PK_DecryptorFilter( rng_, d, new StringSink( plain ) )
        );

    std::array<byte, AES::DEFAULT_KEYLENGTH> return_key;
    for(size_t i = 0; i<AES::DEFAULT_KEYLENGTH; ++i) {
      return_key[i] = static_cast<byte>(plain[i]);
    }
    return return_key;
  }

  std::string encryptSymmetrically( const std::string& plain, std::array<byte, AES::DEFAULT_KEYLENGTH>& key,
      std::array<byte, AES::BLOCKSIZE>& iv )
  {
    std::string cipher;
    rng_.GenerateBlock( key.data(), key.size() );
    rng_.GenerateBlock( iv.data(), iv.size() );

    CFB_Mode<AES>::Encryption e;
    e.SetKeyWithIV( &key[0], key.size(), &iv[0] );

    // CFB mode must not use padding. Specifying
    //  a scheme will result in an exception
    StringSource( plain, true, new StreamTransformationFilter( e, new StringSink( cipher ) ) // StreamTransformationFilter
        );// StringSource
    return cipher;
  }

  std::string decryptSymmetrically( const std::string& cipher, const std::array<byte, AES::DEFAULT_KEYLENGTH>& key,
      const std::array<byte, AES::BLOCKSIZE>& iv )
  {
    std::string recovered;

    CFB_Mode<AES>::Decryption d;
    d.SetKeyWithIV( key.data(), key.size(), iv.data() );

    // The StreamTransformationFilter removes
    //  padding as required.
    StringSource s(cipher, true,
      new StreamTransformationFilter(d,
        new StringSink(recovered)
      ) // StreamTransformationFilter
    ); // StringSource

    return recovered;
  }

protected:
  AutoSeededRandomPool rng_;
  RSA::PublicKey pubk_;
  RSA::PrivateKey privk_;
  std::string pubk_str_;
  std::string fingerprint_;
};
