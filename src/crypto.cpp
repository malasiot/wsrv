#include "detail/crypto.hpp"

#include <cryptopp/config.h>
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/base64.h>

#include <iostream>
#include <iomanip>
#include <cassert>

#include <arpa/inet.h>

using namespace std ;

namespace ws {

string randomBytes(size_t len) {
    using namespace CryptoPP ;
    static AutoSeededRandomPool pool ;
    string bytes ;

    bytes.resize(len, 0) ;

    pool.GenerateBlock(reinterpret_cast<CryptoPP::byte *>(&bytes[0]), len);

    return bytes ;
}

string binToHex(const string &src) {
    using namespace CryptoPP ;

    string res ;
    HexEncoder hex(new StringSink(res));

    hex.Put(reinterpret_cast<const CryptoPP::byte *>(&src[0]), src.size());
    hex.MessageEnd();
    return res ;
}

string encodeBase64(const string &src)
{
    using namespace CryptoPP ;

    string encoded ;

    CryptoPP::StringSource ss(reinterpret_cast<const CryptoPP::byte *>(&src[0]), src.size(), true,
        new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(encoded)
        )
    );

    encoded.pop_back();

    return encoded ;
}

string decodeBase64(const string &src)
{
    using namespace CryptoPP ;

    string decoded ;

    CryptoPP::StringSource ss(reinterpret_cast<const CryptoPP::byte *>(&src[0]), src.size(), true,
        new CryptoPP::Base64Decoder(
            new CryptoPP::StringSink(decoded)
        )
    );

    return decoded ;
}


const size_t salt_length = 16 ;

string passwordHash(const string &password, size_t iterations) {

    using namespace CryptoPP;

    string buffer = randomBytes(salt_length) ;

    // buffer contains salt + key + iterations
    buffer.resize(SHA256::DIGESTSIZE + salt_length + 4) ;

    // generate key from hash and password

    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;

    pbkdf.DeriveKey(
        ( CryptoPP::byte *)buffer.data() + salt_length, SHA256::DIGESTSIZE,
        0x00,
        ( CryptoPP::byte *) password.data(), password.size(),
        ( CryptoPP::byte *) buffer.data(), salt_length,
        iterations
    );

    uint32_t *p = (uint32_t *)(buffer.data() + SHA256::DIGESTSIZE + salt_length) ;

    *p = htonl(iterations) ;

    return buffer ;
}

bool passwordVerify(const string &password, const string &hash) {

    using namespace CryptoPP;

    assert( hash.size() == SHA256::DIGESTSIZE + salt_length + 4 ) ;

    uint32_t *p = (uint32_t *)(hash.data() + SHA256::DIGESTSIZE + salt_length) ;

    size_t iterations = ntohl(*p) ;

    // generate key from hash and password

    char key[SHA256::DIGESTSIZE] ;

    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;

    pbkdf.DeriveKey(
        ( CryptoPP::byte *)key, SHA256::DIGESTSIZE,
        0x00,
        ( CryptoPP::byte *) password.data(), password.size(),
        ( CryptoPP::byte *) hash.data(), salt_length,
        iterations
    );

    // compare key with one stored in hash (avoiding timing issues)

    uint ncount = 0 ;
    for( uint i=0 ; i<SHA256::DIGESTSIZE ; i++ )
        if ( hash.at(salt_length + i) != key[i] ) ncount ++ ;

    return ncount == 0 ;
}

string hashSHA256(const string &src)
{
    std::string digest;
    CryptoPP::SHA256 hash;

    CryptoPP::StringSource ss(reinterpret_cast<const CryptoPP::byte *>(&src[0]), src.size(), true,
       new CryptoPP::HashFilter(hash, new CryptoPP::StringSink(digest)));

    return digest;
}

bool hashCompare(const string &s1, const string &s2)
{
    // compare key with one stored in hash (avoiding timing issues)

    size_t len = std::min(s1.length(), s2.length()) ;
    uint ncount = 0 ;
    for( uint i=0 ; i<len ; i++ )
        if ( s1[i] != s2[i] ) ncount ++ ;

    return ncount == 0 ;
}


} // namespace ws
