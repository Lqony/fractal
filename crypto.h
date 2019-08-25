#pragma once

#include <string>

#include <cryptopp/files.h>
using CryptoPP::FileSink;
using CryptoPP::FileSource;

#include <cryptopp/hex.h>
using CryptoPP::HexEncoder;

#include <cryptopp/filters.h>
using CryptoPP::StringSink;
using CryptoPP::ArraySink;
using CryptoPP::StringSource;
using CryptoPP::PK_EncryptorFilter;
using CryptoPP::PK_DecryptorFilter;
using CryptoPP::SignerFilter;
using CryptoPP::SignatureVerificationFilter;

#include <cryptopp/osrng.h>
using CryptoPP::AutoSeededRandomPool;

#include <cryptopp/integer.h>
using CryptoPP::Integer;

#include <cryptopp/pubkey.h>
using CryptoPP::PublicKey;
using CryptoPP::PrivateKey;

#include <cryptopp/eccrypto.h>
using CryptoPP::ECP;    // Prime field
using CryptoPP::EC2N;   // Binary field
using CryptoPP::SHA256;
using CryptoPP::ECIES;
using CryptoPP::ECDSA;
using CryptoPP::ECPPoint;
using CryptoPP::DL_GroupParameters_EC;
using CryptoPP::DL_GroupPrecomputation;
using CryptoPP::DL_FixedBasePrecomputation;

#include <cryptopp/pubkey.h>
using CryptoPP::DL_PrivateKey_EC;
using CryptoPP::DL_PublicKey_EC;

#include <cryptopp/asn.h>
#include <cryptopp/oids.h>
namespace ASN1 = CryptoPP::ASN1;

#include <cryptopp/cryptlib.h>
using CryptoPP::PK_Encryptor;
using CryptoPP::PK_Decryptor;
using CryptoPP::g_nullNameValuePairs;

#include <cryptopp/keccak.h>
using CryptoPP::Keccak_256;

#include <cryptopp/osrng.h>
using CryptoPP::OS_GenerateRandomBlock;
#include <cryptopp/secblock.h>
using CryptoPP::SecByteBlock;

#include "types.h"

static AutoSeededRandomPool prng;

inline static void set_seed(size_t size)
{
	SecByteBlock seed(size);

	OS_GenerateRandomBlock(false, seed, seed.size());
	prng.IncorporateEntropy(seed, seed.size());
}

inline void encrypt(const key_type pub_key, const blob_type& data, blob_type& res)
{
	ECIES<ECP>::Encryptor encryptor;
	StringSource strs(pub_key, true);
	encryptor.AccessPublicKey().Load(strs);
	StringSource enc_strs(data, true, new PK_EncryptorFilter(prng, encryptor, new StringSink(res)));
}

inline void decrypt(const key_type priv_key, const blob_type& data, blob_type& res)
{
	ECIES<ECP>::Decryptor decryptor;
	StringSource strs(priv_key, true);
	decryptor.AccessPrivateKey().Load(strs);
	StringSource dec_strs(data, true, new PK_DecryptorFilter(prng, decryptor, new StringSink(res)));
}

inline void sign(const key_type priv_key, const blob_type& data, blob_type& res)
{
	ECDSA<ECP, SHA256>::Signer signer;
	StringSource strs(priv_key, true);
	signer.AccessKey().Load(strs);
	StringSource sig_strs(data, true, new SignerFilter(prng, signer, new StringSink(res)));
}

inline bool verify(const key_type pub_key, const blob_type& data, const blob_type& signature)
{
	bool res(false);
	ECDSA<ECP, SHA256>::Verifier verifier;
	StringSource strs(pub_key, true);
	verifier.AccessKey().Load(strs);
	StringSource ver_strs(signature+data, true, new SignatureVerificationFilter(verifier, new ArraySink((byte*)&res, sizeof(res))));

	return res;
}

inline void get_sign_keys(key_type& priv_key, key_type& pub_key)
{
	CryptoPP::StringSink ssd_prv(priv_key);
	CryptoPP::StringSink ssd_pub(pub_key);
	
	ECDSA<ECP, SHA256>::PrivateKey private_key;
	private_key.Initialize( prng, ASN1::secp256r1() );

	ECDSA<ECP, SHA256>::PublicKey public_key;
	private_key.MakePublicKey(public_key);
	
	private_key.Save(ssd_prv);
	public_key.Save(ssd_pub);
}

inline void get_crypt_keys(key_type& priv_key, key_type& pub_key)
{
	CryptoPP::StringSink ssd_prv(priv_key);
	CryptoPP::StringSink ssd_pub(pub_key);
	
	ECIES<ECP>::Decryptor d0(prng, ASN1::secp256r1());
	ECIES<ECP>::Encryptor e0(d0);
	
	d0.GetKey().Save(ssd_prv);
	e0.GetKey().Save(ssd_pub);
}

inline void get_hash(const blob_type& data, hash_type& h)
{
	Keccak_256 hash;
	hash.Update((const byte*)data.data(), data.size());
	h.resize(hash.DigestSize());
	hash.Final((byte*)&h[0]);
}
