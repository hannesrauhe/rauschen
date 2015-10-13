#include "crypto.hpp"
#include "common.hpp"
#include "logger.hpp"

#include <ctime>
#include <chrono>

void Crypto::generate( const std::string& fname )
{
  Logger::info( "Creating key pair" );
  AutoSeededRandomPool rng;
  RSA::PrivateKey privateKey;
  privateKey.GenerateRandomWithKeySize( rng, 3072 );
  RSA::PublicKey publicKey( privateKey );
  save( fname + ".pub", publicKey );
  save( fname, privateKey );
}

std::string Crypto::getFingerprint( const std::string& key_str )
{
  SHA1 hash;
  byte buffer[2 * SHA1::DIGESTSIZE]; // Output size of the buffer
  StringSource f( key_str, true,
      new CryptoPP::HashFilter( hash,
          new CryptoPP::HexEncoder( new CryptoPP::ArraySink( buffer, 2 * SHA1::DIGESTSIZE ) ) ) );
  return std::string( (const char*) ((buffer)), 2 * SHA1::DIGESTSIZE );
}

Crypto::Crypto( const std::string& key_file_path )
{
  load( key_file_path + ".pub", pubk_, &pubk_str_ );
  load( key_file_path, privk_ );
  fingerprint_ = getFingerprint( pubk_str_ );
}

bool Crypto::checkAndEncrypt( const PEncryptedContainer& outer, std::unique_ptr<PInnerContainer>& inner_container )
{
  if ( !outer.has_pubkey() )
  {
    Logger::debug( "invalid message: " + outer.DebugString() );
    return false;
  }
  if ( !outer.has_container() || !outer.has_sym_key() || !outer.has_sym_iv() )
  {
    Logger::debug( "Message was a ping" );
    inner_container.reset();
    return true;
  }
  auto key = decryptKey( outer.sym_key() );
  std::array<byte, AES::BLOCKSIZE> iv;
  for ( auto i = 0; i < AES::BLOCKSIZE; ++i )
  {
    iv[i] = static_cast<unsigned char>( outer.sym_iv()[i] );
  }
  PSignedContainer sig_cont;
  sig_cont.ParseFromString( this->decryptSymmetrically( outer.container(), key, iv ) );
  inner_container.reset( sig_cont.release_inner_cont() );
  bool v_success = verify( inner_container->SerializeAsString(), sig_cont.signature(), outer.pubkey() );
  if ( !v_success )
  {
    Logger::debug( "Verification failed" );
    return false;
  }
  if ( inner_container->receiver() != getFingerprint( pubk_str_ ) )
  {
    Logger::debug( "Message was encrypted with my public key but not meant for me" );
    return false;
  }
  return v_success;
}

PEncryptedContainer Crypto::createEncryptedContainer( const std::string& receiver, const PInnerContainer& inner_cont )
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
    enc_cont.set_sym_key( encryptKey( key, receiver ) );
    enc_cont.set_sym_iv( iv.data(), iv.size() );
  }
  return enc_cont;
}

std::string Crypto::sign( const std::string& data )
{
  std::string signature;
  CryptoPP::RSASS<PSS, SHA1>::Signer signer( privk_ );
  StringSource ss1( data, true, new CryptoPP::SignerFilter( rng_, signer, new StringSink( signature ) ) // SignerFilter
      );
  return signature;
}

bool Crypto::verify( const std::string& data, const std::string& signature_str, const std::string& other_key )
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
	Logger::error("Verfication failed: " + e.GetWhat());
    return false;
  }
  return true;
}

std::string Crypto::encryptKey( const std::array<byte, AES::DEFAULT_KEYLENGTH>& sym_key,
    const std::string& other_key_str )
{
  std::string cipher;
  RSA::PublicKey key;
  CryptoPP::ByteQueue bt;
  StringSource s( other_key_str, true );
  s.CopyTo( bt );
  key.Load( bt );
  RSAES_OAEP_SHA_Encryptor e( key );
  assert( 0 != e.FixedMaxPlaintextLength() );
  assert( sym_key.size() <= e.FixedMaxPlaintextLength() );
  StringSource( sym_key.data(), sym_key.size(), true, new PK_EncryptorFilter( rng_, e, new StringSink( cipher ) ) );
  return cipher;
}

std::array<byte, AES::DEFAULT_KEYLENGTH> Crypto::decryptKey( const std::string& cipher )
{
  std::string plain;
  RSAES_OAEP_SHA_Decryptor d( privk_ );
  StringSource( cipher, true, new PK_DecryptorFilter( rng_, d, new StringSink( plain ) ) );
  std::array<byte, AES::DEFAULT_KEYLENGTH> return_key;
  for ( size_t i = 0; i < AES::DEFAULT_KEYLENGTH; ++i )
  {
    return_key[i] = static_cast<byte>( plain[i] );
  }
  return return_key;
}

std::string Crypto::encryptSymmetrically( const std::string& plain, std::array<byte, AES::DEFAULT_KEYLENGTH>& key,
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
      );
  return cipher;
}

std::string Crypto::decryptSymmetrically( const std::string& cipher,
    const std::array<byte, AES::DEFAULT_KEYLENGTH>& key, const std::array<byte, AES::BLOCKSIZE>& iv )
{
  std::string recovered;
  CFB_Mode<AES>::Decryption d;
  d.SetKeyWithIV( key.data(), key.size(), iv.data() );
  // The StreamTransformationFilter removes
  //  padding as required.
  StringSource s( cipher, true, new StreamTransformationFilter( d, new StringSink( recovered ) ) // StreamTransformationFilter
      );
  return recovered;
}
