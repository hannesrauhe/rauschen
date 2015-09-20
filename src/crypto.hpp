#pragma once

#include <fstream>
#include <memory>
#include <gcrypt.h>
#include <iomanip>
#include <sstream>
#include "logger.hpp"

class PubKey
{

};

class Crypto
{
public:
  static void gcryptInit()
  {
    if ( !gcry_check_version( GCRYPT_VERSION ) )
    {
      throw std::runtime_error( "gcrypt: library version mismatch" );
    }

    gcry_error_t err = 0;

    /* We don't want to see any warnings, e.g. because we have not yet
     parsed program options which might be used to suppress such
     warnings. */
    err = gcry_control( GCRYCTL_SUSPEND_SECMEM_WARN );

    /* ... If required, other initialization goes here.  Note that the
     process might still be running with increased privileges and that
     the secure memory has not been intialized.  */

    /* Allocate a pool of 16k secure memory.  This make the secure memory
     available and also drops privileges where needed.  */
    err |= gcry_control( GCRYCTL_INIT_SECMEM, 16384, 0 );

    /* It is now okay to let Libgcrypt complain when there was/is
     a problem with the secure memory. */
    err |= gcry_control( GCRYCTL_RESUME_SECMEM_WARN );

    /* Tell Libgcrypt that initialization has completed. */
    err |= gcry_control( GCRYCTL_INITIALIZATION_FINISHED, 0 );

    if ( err )
    {
      throw std::runtime_error( "gcrypt: problem when initializing secure memory" );
    }
  }

  static void generate( const std::string& fname )
  {
    gcry_error_t err = 0;
    gcry_sexp_t rsa_keypair;

    std::ofstream lockf( fname );
    if ( !lockf.good() )
    {
      throw std::runtime_error( "fopen() failed" );
    }

    /* Generate a new RSA key pair. */
    Logger::info( "RSA key generation can take a few minutes. Your computer \n"
        "needs to gather random entropy. Please wait..." );

    gcry_sexp_t rsa_parms;

    err = gcry_sexp_build( &rsa_parms, NULL, "(genkey (rsa (nbits 4:2048)))" );
    if ( err )
    {
      throw std::runtime_error( "gcrypt: failed to create rsa params" );
    }

    err = gcry_pk_genkey( &rsa_keypair, rsa_parms );
    if ( err )
    {
      throw std::runtime_error( "gcrypt: failed to create rsa key pair" );
    }

    auto length = gcry_sexp_sprint( rsa_keypair, GCRYSEXP_FMT_DEFAULT, nullptr, 0 );

    std::unique_ptr<char[]> rsa_buf( new char[length] );
    gcry_sexp_sprint( rsa_keypair, GCRYSEXP_FMT_DEFAULT, rsa_buf.get(), length );

    lockf.write( rsa_buf.get(), length );

    /* Release contexts. */
    gcry_sexp_release( rsa_parms );
  }

  static std::string getFingerprint( const std::string& key_str )
  {
    const static int algo = GCRY_MD_SHA1;
    auto digest_length = gcry_md_get_algo_dlen( algo );
    unsigned char* data = new unsigned char[digest_length];
    gcry_md_hash_buffer( algo, data, key_str.data(), key_str.length() );

    std::string buffer( digest_length * 2, 'X' );
    const char* pszNibbleToHex = { "0123456789ABCDEF" };
    int nNibble, i;
    for ( i = 0; i < digest_length; i++ )
    {
      nNibble = data[i] >> 4;
      buffer[2 * i] = pszNibbleToHex[nNibble];
      nNibble = data[i] & 0x0F;
      buffer[2 * i + 1] = pszNibbleToHex[nNibble];
    }
    delete[] data;
    return buffer;
  }

  Crypto( const std::string& key_file_path )
  {
    gcry_error_t err = 0;
    gcry_sexp_t rsa_keypair;
    std::ifstream keypair_file( key_file_path );
    if ( !keypair_file.good() )
    {
      throw std::runtime_error( "fopen() failed" );
    }

    keypair_file.seekg( 0, std::ios::end );
    size_t file_size = keypair_file.tellg();

    if(file_size==0) {
      throw std::runtime_error( "keyfile is empty" );
    }
    std::unique_ptr<char[]> rsabuf( new char[file_size] );
    keypair_file.seekg( 0, std::ios::beg );
    keypair_file.read( rsabuf.get(), file_size );

    size_t erroff;
    err = gcry_sexp_new( &rsa_keypair, rsabuf.get(), file_size, 1 );
    if ( err )
    {
      throw std::runtime_error( "Could not read keypair" );
    }

    pubk_ = gcry_sexp_find_token( rsa_keypair, "public-key", 0 );
    privk_ = gcry_sexp_find_token( rsa_keypair, "private-key", 0 );
    if ( pubk_ == nullptr )
    {
      throw std::runtime_error( "Pubkey not found in keypair" );
    }
    if ( privk_ == nullptr )
    {
      throw std::runtime_error( "Privkey not found in keypair" );
    }

    gcry_sexp_release( rsa_keypair );

    auto length = gcry_sexp_sprint( pubk_, GCRYSEXP_FMT_DEFAULT, nullptr, 0 );

    std::unique_ptr<char[]> pubk_str( new char[length] );
    gcry_sexp_sprint( pubk_, GCRYSEXP_FMT_DEFAULT, pubk_str.get(), length );
    pubk_str_.assign( pubk_str.get(), length );
    fingerprint_ = getFingerprint( pubk_str_ );
  }

  const std::string& getPubKey() const
  {
    return pubk_str_;
  }

  std::string sign( const std::string& data) {
    return "";
  }

  std::string encrypt( const std::string& data, const std::string& other_key )
  {
    return "";
  }

  ~Crypto()
  {
    gcry_sexp_release( pubk_ );
    gcry_sexp_release( privk_ );
  }

  size_t getKeypairSize( int nbits )
  {
    size_t aes_blklen = gcry_cipher_get_algo_blklen( GCRY_CIPHER_AES128 );

    // format overhead * {pub,priv}key (2 * bits)
    size_t keypair_nbits = 4 * (2 * nbits);

    size_t rem = keypair_nbits % aes_blklen;
    return (keypair_nbits + rem) / 8;
  }
protected:
  gcry_sexp_t pubk_;
  gcry_sexp_t privk_;
  std::string pubk_str_;
  std::string fingerprint_;
};
